#include "Framework.h"
#include "Camera.h"
#include "Transform.h"


Camera::Camera(Context * context, Actor * actor, Transform * transform)
	: IComponent(context, actor, transform)
	, projectionType(ProjectionType::Perspective)
	, fov(static_cast<float>(D3DXToRadian(45)))
	, nearPlane(0.3f)
	, farPlane(1000.0f)
	, bEditorCamera(false)
	, acceleration(3.0f)
	, drag(acceleration * 0.8f)
	, movement_speed(0, 0, 0)
{
	input = nullptr;
	timer = nullptr;

	this->transform->Translate(D3DXVECTOR3(-37.85f,146.98f,-80.48f));
	this->transform->SetRotationFromEulerAngle(D3DXVECTOR3(36.2f,41.7f,0.00f));
	
	D3DXMatrixIdentity(&view_base);
	D3DXMatrixIdentity(&view);
	D3DXMatrixIdentity(&proj);
}

Camera::Camera(Context * context)
	: IComponent(context, nullptr, nullptr)
	, projectionType(ProjectionType::Perspective)
	, fov(static_cast<float>(D3DXToRadian(45)))
	, nearPlane(0.3f)
	, farPlane(1000.0f)
	, bEditorCamera(true)
	, acceleration(1.0f)
	, drag(acceleration * 0.8f)
	, movement_speed(0, 0, 0)
{
	input = context->GetSubsystem<Input>();
	timer = context->GetSubsystem<Timer>();
	renderer = context->GetSubsystem<Renderer>();

	transform = new Transform(context, nullptr, nullptr);

	transform->Translate(D3DXVECTOR3(-37.85f, 146.98f, -80.48f));
	transform->SetRotationFromEulerAngle(D3DXVECTOR3(36.2f, 41.7f, 0.00f));

	D3DXMatrixIdentity(&view_base);
	D3DXMatrixIdentity(&view);
	D3DXMatrixIdentity(&proj);
}

Camera::~Camera()
{
	if (bEditorCamera)
		SAFE_DELETE(transform);
}

void Camera::Serialize(FileStream * stream)
{
	stream->Write(static_cast<uint>(projectionType));
	stream->Write(fov);
	stream->Write(nearPlane);
	stream->Write(farPlane);
}

void Camera::Deserialize(FileStream * stream)
{
	projectionType = static_cast<ProjectionType>(stream->Read<uint>());
	stream->Read(fov);
	stream->Read(nearPlane);
	stream->Read(farPlane);

	UpdateViewBaseMatrix();
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void Camera::OnInitialize()
{
}

void Camera::OnStart()
{
}

void Camera::OnUpdate()
{
	UpdateViewBaseMatrix();
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void Camera::OnStop()
{
}

void Camera::OnDestroy()
{
}

auto Camera::GetViewProjectionInverseMatrix() const -> const D3DXMATRIX
{
	D3DXMATRIX inverse = GetViewProjectionMatrix();
	D3DXMatrixInverse(&inverse, nullptr, &inverse);
	return inverse;
}

auto Camera::WorldToScreenPoint(const D3DXVECTOR3 & world_position) const -> const D3DXVECTOR2
{
	const auto& resolution = context->GetSubsystem<Renderer>()->GetResolution();

	D3DXMATRIX projection;
	D3DXMatrixPerspectiveFovLH(&projection, fov, resolution.x / resolution.y, nearPlane, farPlane);

	D3DXVECTOR3 clip_position;
	D3DXVec3TransformCoord(&clip_position, &world_position, &(view * projection));

	D3DXVECTOR2 screen_position;
	screen_position.x = (clip_position.x / clip_position.z) * (0.5f * resolution.x) + (0.5f * resolution.x);
	screen_position.y = (clip_position.y / clip_position.z) * -(0.5f * resolution.y) + (0.5f * resolution.y);

	return screen_position;
}

auto Camera::ScreenToWorldPoint(const D3DXVECTOR2 & screen_position) const -> const D3DXVECTOR3
{
	const auto& resolution = context->GetSubsystem<Renderer>()->GetResolution();

	D3DXVECTOR3 clip_position;
	clip_position.x = (screen_position.x / resolution.x) * 2.0f - 1.0f;
	clip_position.y = (screen_position.y / resolution.y) * -2.0f + 1.0f;
	clip_position.z = 1.0f;

	D3DXMATRIX unprojection;
	D3DXMatrixInverse(&unprojection, nullptr, &(view * proj));

	D3DXVECTOR3 world_position;
	D3DXVec3TransformCoord(&world_position, &clip_position, &unprojection);

	return world_position;
}

void Camera::GetWorldRay(D3DXVECTOR3 & origin, D3DXVECTOR3 & direction)
{

	D3DXVECTOR2 mousePos;
	D3DXVECTOR2 resolution;
	renderer->GetCursorPos(mousePos);
	resolution = renderer->GetResolution();
	//Projection

	//ERASE
	LOG_INFO_F("mousePos %0.2f %0.2f, resolution %0.2f %0.2f", mousePos.x, mousePos.y, resolution.x, resolution.y);
	//


	D3DXVECTOR2 point;
	point.x = +2.0f * mousePos.x / resolution.x - 1.0f;
	point.y = -2.0f * mousePos.y / resolution.y + 1.0f;
	point.x = point.x / proj._11;
	point.y = point.y / proj._22;

	//View
	D3DXMATRIX invView;
	D3DXMatrixInverse(&invView, nullptr, &view);

	D3DXVECTOR3 org = transform->GetTranslation();
	D3DXVECTOR3 dir = D3DXVECTOR3(point.x, point.y, 1.0f);
	D3DXVec3TransformNormal(&dir, &dir, &invView);
	D3DXVec3Normalize(&dir, &dir);

	origin = org;
	direction = dir;
}

void Camera::GetLocalRay(D3DXVECTOR3 & origin, D3DXVECTOR3 & direction, const D3DXMATRIX & world)
{
	D3DXVECTOR3 org, dir;
	GetWorldRay(org, dir);

	D3DXMATRIX invWorld;
	D3DXMatrixInverse(&invWorld, nullptr, &world);

	D3DXVec3TransformNormal(&dir, &dir, &invWorld);
	D3DXVec3Normalize(&dir, &dir);

	origin = org;
	direction = dir;
}

void Camera::UpdateEditorCamera()
{
	if (!bEditorCamera)
		return;

	D3DXVECTOR3 rotation = transform->GetEulerAngle();
	D3DXVECTOR3 right = transform->GetRight();
	D3DXVECTOR3 up = transform->GetUp();
	D3DXVECTOR3 forward = transform->GetForward();

	if (input->KeyPress(KeyCode::KEY_SHIFT))
	{
		if (input->KeyPress(KeyCode::KEY_W))
			movement_speed += forward * acceleration * timer->GetDeltaTimeSec();
		else if (input->KeyPress(KeyCode::KEY_S))
			movement_speed -= forward * acceleration * timer->GetDeltaTimeSec();

		if (input->KeyPress(KeyCode::KEY_A))
			movement_speed -= right * acceleration * timer->GetDeltaTimeSec();
		else if (input->KeyPress(KeyCode::KEY_D))
			movement_speed += right * acceleration * timer->GetDeltaTimeSec();

		if (input->KeyPress(KeyCode::KEY_E))
			movement_speed += up * acceleration * timer->GetDeltaTimeSec();
		else if (input->KeyPress(KeyCode::KEY_Q))
			movement_speed -= up * acceleration * timer->GetDeltaTimeSec();

		if (input->BtnPress(KeyCode::CLICK_RIGHT))
		{
			auto moveValue = input->GetMouseMoveValue();

			rotation.x += moveValue.y * 0.1f;
			rotation.y += moveValue.x * 0.1f;

			rotation.x = Math::Clamp(rotation.x, -90.0f, 90.0f);

			transform->Translate(movement_speed);
			transform->SetRotationFromEulerAngle(rotation);
		}
	}

	movement_speed *= drag * (1.0f - timer->GetDeltaTimeSec());

	D3DXVECTOR3 position = transform->GetTranslation();
	LOG_INFO_F("Camear Position : %0.2f %0.2f %0.2f", position.x, position.y, position.z);
	LOG_INFO_F("Camera Rotation : %0.2f %0.2f %0.2f", rotation.x, rotation.y, rotation.z);

	UpdateViewBaseMatrix();
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void Camera::UpdateViewBaseMatrix()
{
	D3DXMatrixLookAtLH
	(
		&view_base,
		&D3DXVECTOR3(0, 0, -nearPlane),
		&D3DXVECTOR3(0, 0, 1),
		&D3DXVECTOR3(0, 1, 0)
	);
}

void Camera::UpdateViewMatrix()
{
	D3DXVECTOR3 position = transform->GetTranslation();
	D3DXVECTOR3 up = transform->GetUp();
	D3DXVECTOR3 forward = transform->GetForward();

	D3DXMatrixLookAtLH(&view, &position, &(position + forward), &up);
}

void Camera::UpdateProjectionMatrix()
{
	auto resolution = context->GetSubsystem<Renderer>()->GetResolution();
	auto zn = Settings::Get().IsReverseZ() ? farPlane : nearPlane;
	auto zf = Settings::Get().IsReverseZ() ? nearPlane : farPlane;

	switch (projectionType)
	{
	case ProjectionType::Perspective:
		D3DXMatrixPerspectiveFovLH
		(
			&proj,
			fov,
			resolution.x / resolution.y,
			zn,
			zf
		);
		break;
	case ProjectionType::Orthographic:
		break;
	}
}
