#include "Framework.h"
#include "Light.h"
#include "../Camera.h"
#include "../Transform.h"

Light::Light(Context * context, Actor * actor, Transform * transform)
	: IComponent(context, actor, transform)
	, light_type(LightType::Point)
	, range(10.0f)
	, intensity(2.0f)
	, radian_angle(0.5f)
	, bias(0.0008f)
	, normal_bias(120.0f)
	, color(1.0f, 0.76f, 0.57f, 1.0f)
	, last_camera_position(0, 0, 0)
	, last_light_position(0, 0, 0)
	, last_light_rotation(0, 0, 0, 1)
	, bCast_shadow(true)
	, bUpdate(true)
{
	views.reserve(6);
	views.resize(6);

	projections.reserve(6);
	projections.resize(6);

	renderer = context->GetSubsystem<Renderer>();
}

Light::~Light()
{
}

void Light::Serialize(FileStream * stream)
{
	stream->Write(static_cast<uint>(light_type));
	stream->Write(bCast_shadow);
	stream->Write(color);
	stream->Write(range);
	stream->Write(intensity);
	stream->Write(radian_angle);
	stream->Write(bias);
	stream->Write(normal_bias);
}

void Light::Deserialize(FileStream * stream)
{
	SetLightType(static_cast<LightType>(stream->Read<uint>()));
	stream->Read(bCast_shadow);
	stream->Read(color);
	stream->Read(range);
	stream->Read(intensity);
	stream->Read(radian_angle);
	stream->Read(bias);
	stream->Read(normal_bias);
}

void Light::OnInitialize()
{
}

void Light::OnStart()
{
}

void Light::OnUpdate()
{
}

void Light::OnStop()
{
}

void Light::OnDestroy()
{
}

auto Light::GetDirection() const -> D3DXVECTOR3
{
	return transform->GetForward();
}

void Light::SetLightType(const LightType & type)
{
	if (this->light_type == type)
		return;

	this->light_type = type;
	this->bUpdate = true;

	CreateShadowMap(true);
}

void Light::SetRange(const float & range)
{
	this->range = Math::Clamp(range, 0.0f, std::numeric_limits<float>::infinity());
}

void Light::SetAngle(const float & angle)
{
	this->radian_angle = Math::Clamp(angle, 0.0f, 1.0f);
	this->bUpdate = true;
}

void Light::SetIsCastShadow(const bool & bCast_shadow)
{
	if (this->bCast_shadow == bCast_shadow)
		return;

	this->bCast_shadow = bCast_shadow;

	CreateShadowMap(true);
}

auto Light::GetViewMatrix(const uint & index) -> const D3DXMATRIX
{
	if (index >= views.size())
		return *D3DXMatrixIdentity(&D3DXMATRIX());

	return views[index];
}

auto Light::GetProjectionMatrix(const uint & index) -> const D3DXMATRIX
{
	if (index >= projections.size())
		return *D3DXMatrixIdentity(&D3DXMATRIX());

	return projections[index];
}

void Light::ClampRotation()
{
	auto rotation = transform->GetEulerAngle();

	if (rotation.x <= 0.0f)
	{
		D3DXQUATERNION quaternion;
		D3DXQuaternionRotationYawPitchRoll
		(
			&quaternion,
			rotation.y * Math::TO_RAD,
			179.0f,
			rotation.z * Math::TO_RAD
		);
		transform->SetRotation(quaternion);
	}

	if (rotation.x >= 180.0f)
	{
		D3DXQUATERNION quaternion;
		D3DXQuaternionRotationYawPitchRoll
		(
			&quaternion,
			rotation.y * Math::TO_RAD,
			1.0f,
			rotation.z * Math::TO_RAD
		);
		transform->SetRotation(quaternion);
	}
}

void Light::ComputeViewMatrix()
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 look_at;
	D3DXVECTOR3 up;

	switch (light_type)
	{
	case LightType::Directional:
	{
		auto direction = GetDirection();
		position = direction;
		look_at = position + direction;
		up = D3DXVECTOR3(0, 1, 0);

		views[0] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &look_at, &up);
		views[1] = views[0];
		views[2] = views[0];
		break;
	}
	case LightType::Point:
	{
		position = transform->GetTranslation();

		views[0] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &(position + D3DXVECTOR3(+1, +0, +0)), &D3DXVECTOR3(+0, +1, +0)); //x+
		views[1] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &(position + D3DXVECTOR3(-1, +0, +0)), &D3DXVECTOR3(+0, +1, +0)); //x-
		views[2] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &(position + D3DXVECTOR3(+0, +1, +0)), &D3DXVECTOR3(+0, +0, -1)); //y+
		views[3] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &(position + D3DXVECTOR3(+0, -1, +0)), &D3DXVECTOR3(+0, +0, +1)); //y-
		views[4] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &(position + D3DXVECTOR3(+0, +0, +1)), &D3DXVECTOR3(+0, +1, +0)); //z+
		views[5] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &(position + D3DXVECTOR3(+0, +0, -1)), &D3DXVECTOR3(+0, +1, +0)); //z-

		break;
	}
	case LightType::Spot:
	{
		position = transform->GetTranslation();
		look_at = transform->GetForward();
		up = transform->GetUp();

		look_at += position;

		views[0] = *D3DXMatrixLookAtLH(&D3DXMATRIX(), &position, &look_at, &up);
		break;
	}
	}

}

auto Light::ComputeProjectionMatrix(const uint & index) -> const bool
{
	if (!renderer->GetMainCamera() || index >= shadow_map->GetArraySize())
		return false;

	const auto& camera = renderer->GetMainCamera();
	const auto& camera_transform = camera->GetTransform();

	if (light_type == LightType::Directional)
	{
		float splits[3]
		{
			camera->GetFarPlane() * 0.01f,
			camera->GetFarPlane() * 0.05f,
			camera->GetFarPlane(),
		};

		auto split = splits[index];
		auto extent = split * tan(camera->GetFOV() * 0.5f);

		D3DXVECTOR3 box_center = camera_transform->GetTranslation() + camera_transform->GetForward() * split * 0.5f;
		D3DXVec3TransformCoord(&box_center, &box_center, &GetViewMatrix());

		D3DXVECTOR3 box_extent = D3DXVECTOR3(extent, extent, extent);
		D3DXVec3TransformCoord(&box_extent, &box_extent, &transform->GetWorldRotationMatrix());

		D3DXVECTOR3 box_min = box_center - box_extent;
		D3DXVECTOR3 box_max = box_center + box_extent;

		// https://msdn.microsoft.com/en-us/library/windows/desktop/ee416324(v=vs.85).aspx

		float unit_per_texel = (extent * 2.0f) / static_cast<float>(shadow_map->GetWidth());

		box_min /= unit_per_texel;
		box_min = Math::Floor(box_min);
		box_min *= unit_per_texel;

		box_max /= unit_per_texel;
		box_max = Math::Floor(box_max);
		box_max *= unit_per_texel;

		if (Settings::Get().IsReverseZ())
			projections[index] = *D3DXMatrixOrthoOffCenterLH(&D3DXMATRIX(), box_min.x, box_max.x, box_min.y, box_max.y, box_max.z, box_min.z);
		else
			projections[index] = *D3DXMatrixOrthoOffCenterLH(&D3DXMATRIX(), box_min.x, box_max.x, box_min.y, box_max.y, box_min.z, box_max.z);
	}
	else
	{
		const auto width = static_cast<float>(shadow_map->GetWidth());
		const auto height = static_cast<float>(shadow_map->GetHeight());
		const auto aspect_ratio = width / height;
		const auto fov = light_type == LightType::Spot ? radian_angle : Math::PI_DIV_2;
		const auto near_plane = Settings::Get().IsReverseZ() ? range : 0.1f;
		const auto far_plane = Settings::Get().IsReverseZ() ? 0.1f : range;

		D3DXMatrixPerspectiveFovLH(&projections[index], fov, aspect_ratio, near_plane, far_plane);
	}

	return true;
}

void Light::CreateShadowMap(const bool & force)
{
	if (!force && !shadow_map)
		return;

	uint resolution = 4096;

	switch (light_type)
	{
	case LightType::Directional:
		shadow_map = std::make_unique<RenderTexture>(context);
		shadow_map->Create(resolution, resolution, true, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT, 3);
		break;
	case LightType::Point:
		shadow_map = std::make_unique<RenderTexture>(context);
		shadow_map->Create(resolution, resolution, true, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT, 6, true);
		break;
	case LightType::Spot:
		shadow_map = std::make_unique<RenderTexture>(context);
		shadow_map->Create(resolution, resolution, true, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT);
		break;
	}
}
