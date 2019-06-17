#include "Framework.h"
#include "Transform.h"
#include "../Scene.h"
#include "../Actor.h"

Transform::Transform(Context * context, Actor * actor, Transform * transform)
	: IComponent(context, actor, transform)
	, localScale(1, 1, 1)
	, localRotation(0, 0, 0, 1)
	, localTranslation(0, 0, 0)
	, parent(nullptr)
{
	ZeroMemory(&cpu_transform_buffer, sizeof(TransformData));

	D3DXMatrixIdentity(&local);
	D3DXMatrixIdentity(&world);
	D3DXMatrixIdentity(&wvp_previous);
}

void Transform::Serialize(FileStream * stream)
{
	stream->Write(localScale);
	stream->Write(localRotation);
	stream->Write(localTranslation);
	stream->Write(parent ? parent->GetActor()->GetID() : NOT_ASSIGNED_ID);
}

void Transform::Deserialize(FileStream * stream)
{
	stream->Read(localScale);
	stream->Read(localRotation);
	stream->Read(localTranslation);
	uint id = stream->Read<uint>();

	if (id != NOT_ASSIGNED_ID)
	{
		if (const auto p = context->GetSubsystem<SceneManager>()->GetCurrentScene()->GetActorFromID(id))
			p->GetTransform()->AddChild(this);
	}

	UpdateTransform();
}

void Transform::OnInitialize()
{
}

void Transform::OnStart()
{
}

void Transform::OnUpdate()
{
}

void Transform::OnStop()
{
}

void Transform::OnDestroy()
{
}

void Transform::SetLocalScale(const D3DXVECTOR3 & vec)
{
	if (localScale == vec)
		return;

	localScale = vec;
	UpdateTransform();
}

void Transform::SetLocalRotation(const D3DXQUATERNION & qut)
{
	if (localRotation == qut)
		return;

	localRotation = qut;
	UpdateTransform();
}

void Transform::SetLocalTranslation(const D3DXVECTOR3 & vec)
{
	if (localTranslation == vec)
		return;

	localTranslation = vec;
	UpdateTransform();
}

const D3DXVECTOR3 Transform::GetScale()
{
	D3DXVECTOR3 scale;
	D3DXMatrixDecompose
	(
		&scale,
		&D3DXQUATERNION(),
		&D3DXVECTOR3(),
		&world
	);

	return scale;
}

const D3DXQUATERNION Transform::GetRotation()
{
	D3DXQUATERNION rotation;
	D3DXMatrixDecompose
	(
		&D3DXVECTOR3(),
		&rotation,
		&D3DXVECTOR3(),
		&world
	);

	return rotation;
}

const D3DXVECTOR3 Transform::GetTranslation()
{
	return D3DXVECTOR3(world._41, world._42, world._43);
}

const D3DXMATRIX Transform::GetWorldRotationMatrix()
{
	D3DXMATRIX R;
	D3DXQUATERNION qut = GetRotation();
	D3DXMatrixRotationQuaternion(&R, &qut);

	return R;
}

void Transform::SetScale(const D3DXVECTOR3 & vec)
{
	if (GetScale() == vec)
		return;

	if (HasParent())
	{
		D3DXVECTOR3 parentScale = parent->GetScale();
		D3DXVECTOR3 scale;
		scale.x = vec.x / parentScale.x;
		scale.y = vec.y / parentScale.y;
		scale.z = vec.z / parentScale.z;

		SetLocalScale(scale);
	}
	else
		SetLocalScale(vec);
}

void Transform::SetRotation(const D3DXQUATERNION & qut)
{
	if (GetRotation() == qut)
		return;

	if (HasParent())
	{
		D3DXQUATERNION invQut;
		D3DXQuaternionInverse(&invQut, &parent->GetRotation());

		D3DXQUATERNION quaternion = qut * invQut;

		SetLocalRotation(quaternion);
	}
	else
		SetLocalRotation(qut);
}

void Transform::SetRotationFromEulerAngle(const D3DXVECTOR3 & eulerAngle)
{
	D3DXQUATERNION qut;
	D3DXQuaternionRotationYawPitchRoll
	(
		&qut,
		Math::ToRadian(eulerAngle.y),
		Math::ToRadian(eulerAngle.x),
		Math::ToRadian(eulerAngle.z)
	);

	SetRotation(qut);
}

void Transform::SetTranslation(const D3DXVECTOR3 & vec)
{
	if (GetTranslation() == vec)
		return;

	if (HasParent())
	{
		D3DXMATRIX inv;
		D3DXMatrixInverse(&inv, nullptr, &parent->GetWorldMatrix());

		D3DXVECTOR3 position;
		D3DXVec3TransformCoord(&position, &vec, &inv);

		SetLocalTranslation(position);
	}
	else
		SetLocalTranslation(vec);
}

const D3DXVECTOR3 Transform::GetRight()
{
	D3DXVECTOR3 right(1, 0, 0);
	D3DXVec3TransformNormal(&right, &right, &world);

	return right;
}

const D3DXVECTOR3 Transform::GetUp()
{
	D3DXVECTOR3 up(0, 1, 0);
	D3DXVec3TransformNormal(&up, &up, &world);

	return up;
}

const D3DXVECTOR3 Transform::GetForward()
{
	D3DXVECTOR3 forward(0, 0, 1);
	D3DXVec3TransformNormal(&forward, &forward, &world);

	return forward;
}

void Transform::Translate(const D3DXVECTOR3 & delta)
{
	if (HasParent())
	{
		D3DXMATRIX inv;
		D3DXMatrixInverse(&inv, nullptr, &parent->GetWorldMatrix());

		D3DXVECTOR3 position;
		D3DXVec3TransformCoord(&position, &delta, &inv);

		SetLocalTranslation(localTranslation + position);
	}
	else
		SetLocalTranslation(localTranslation + delta);
}

auto Transform::GetChildFromIndex(const uint & index) -> Transform *
{
	if (!HasChilds())
		return nullptr;

	if (index >= GetChildCount())
		return nullptr;

	return childs[index];
}

void Transform::SetParent(Transform * newParent)
{
	if (!newParent)
		return;

	if (newParent->GetID() == GetID())
		return;

	if (HasParent())
	{
		if (parent->GetID() == newParent->GetID())
			return;
	}

	if (newParent->IsChild(this))
	{
		if (HasParent())
		{
			for (auto& child : childs)
				child->SetParent(parent);
		}
		else
		{
			for (auto& child : childs)
				child->Detatch();
		}
	}

	auto oldParent = parent;
	parent = newParent;

	if (oldParent)
		oldParent->AcquireChilds();

	if (parent)
		parent->AcquireChilds();

	UpdateTransform();
}

void Transform::AddChild(Transform * child)
{
	if (!child)
		return;

	if (child->GetID() == GetID())
		return;

	child->SetParent(this);
}

void Transform::Detatch()
{
	if (!HasParent())
		return;

	auto tempParent = parent;
	parent = nullptr;

	UpdateTransform();
	tempParent->AcquireChilds();
}

void Transform::AcquireChilds()
{
	childs.clear();
	childs.shrink_to_fit();

	auto scene = context->GetSubsystem<SceneManager>()->GetCurrentScene();
	if (scene)
	{
		auto actors = scene->GetActors();
		for (auto& actor : actors)
		{
			auto child = actor->GetTransform();

			if (!child->HasParent())
				continue;

			if (child->GetParent()->GetID() == GetID())
			{
				childs.emplace_back(child);
				child->AcquireChilds();
			}
		}
	}
}

const bool Transform::IsChild(Transform * transform)
{
	auto tempChilds = transform->GetChilds();
	for (auto& child : tempChilds)
	{
		if (child->GetID() == GetID())
			return true;
	}
	return false;
}

const D3DXVECTOR3 Transform::GetEulerAngle()
{
	float x = localRotation.x;
	float y = localRotation.y;
	float z = localRotation.z;
	float w = localRotation.w;

	// Derivation from http://www.geometrictools.com/Documentation/EulerAngles.pdf
	// Order of rotations: Z first, then X, then Y
	float check = 2.0f * (-y * z + w * x);

	if (check < -0.995f)
	{
		return D3DXVECTOR3
		(
			-90.0f,
			0.0f,
			-atan2f(2.0f * (x * z - w * y), 1.0f - 2.0f * (y * y + z * z)) * Math::TO_DEG
		);
	}

	if (check > 0.995f)
	{
		return D3DXVECTOR3
		(
			90.0f,
			0.0f,
			atan2f(2.0f * (x * z - w * y), 1.0f - 2.0f * (y * y + z * z)) * Math::TO_DEG
		);
	}

	return D3DXVECTOR3
	(
		asinf(check) * Math::TO_DEG,
		atan2f(2.0f * (x * z + w * y), 1.0f - 2.0f * (x * x + y * y)) * Math::TO_DEG,
		atan2f(2.0f * (x * y + w * z), 1.0f - 2.0f * (x * x + z * z)) * Math::TO_DEG
	);
}

void Transform::UpdateConstantBuffer(const D3DXMATRIX & vp_matrix)
{
	if (!gpu_transform_buffer)
	{
		gpu_transform_buffer = std::make_shared<ConstantBuffer>(context);
		gpu_transform_buffer->Create<TransformData>();
	}

	auto wvp_current = world * vp_matrix;

	auto data = gpu_transform_buffer->Map<TransformData>();
	if (!data)
	{
		LOG_ERROR("Invalid to buffer");
		return;
	}

	D3DXMatrixTranspose(&data->World, &world);
	D3DXMatrixTranspose(&data->WVP_Current, &wvp_current);
	D3DXMatrixTranspose(&data->WVP_Previous, &wvp_previous);

	gpu_transform_buffer->Unmap();

	wvp_previous = wvp_current;
}

void Transform::UpdateTransform()
{
	D3DXMATRIX S, R, T;
	D3DXMatrixScaling(&S, localScale.x, localScale.y, localScale.z);
	D3DXMatrixRotationQuaternion(&R, &localRotation);
	D3DXMatrixTranslation(&T, localTranslation.x, localTranslation.y, localTranslation.z);

	local = S * R * T;

	if (HasParent())
		world = local * parent->GetWorldMatrix();
	else
		world = local;

	for (const auto& child : childs)
		child->UpdateTransform();
}
