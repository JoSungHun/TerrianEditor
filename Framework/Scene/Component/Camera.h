#pragma once
#include "IComponent.h"

enum class ProjectionType : uint
{
	Perspective,
	Orthographic,
};

class Camera final : public IComponent
{
public:
	Camera
	(
		class Context* context,
		class Actor* actor,
		class Transform* transform
	);
	Camera(class Context* context);
	~Camera();

	Camera(const Camera& rhs) = delete;
	Camera& operator=(const Camera& rhs) = delete;

	void Serialize(FileStream* stream) override;
	void Deserialize(FileStream* stream) override;

	void OnInitialize() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;
	void OnDestroy() override;

	auto GetViewBaseMatrix() const -> const D3DXMATRIX& { return view_base; }
	auto GetViewMatrix() const -> const D3DXMATRIX& { return view; }
	auto GetProjectionMatrix() const -> const D3DXMATRIX& { return proj; }
	auto GetViewProjectionMatrix() const -> const D3DXMATRIX { return view * proj; }
	auto GetViewProjectionInverseMatrix() const -> const D3DXMATRIX;

	auto GetProjectionType() const -> const ProjectionType& { return projectionType; }
	void SetProjectionType(const ProjectionType& type) { this->projectionType = type; }

	auto GetNearPlane() const -> const float& { return nearPlane; }
	void SetNearPlane(const float& nearPlane) { this->nearPlane = nearPlane; }

	auto GetFarPlane() const -> const float& { return farPlane; }
	void SetFarPlane(const float& farPlane) { this->farPlane = farPlane; }

	auto GetFOV() const -> const float& { return fov; }
	void SetFOV(const float& fov) { this->fov = fov; }

	auto WorldToScreenPoint(const D3DXVECTOR3& world_position) const -> const D3DXVECTOR2;
	auto ScreenToWorldPoint(const D3DXVECTOR2& screen_position) const -> const D3DXVECTOR3;

	void GetWorldRay(D3DXVECTOR3& origin, D3DXVECTOR3& direction);
	void GetLocalRay(D3DXVECTOR3& origin, D3DXVECTOR3& direction, const D3DXMATRIX& world);


	void UpdateEditorCamera();

private:
	void UpdateViewBaseMatrix();
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

private:
	class Timer* timer;
	class Input* input;
	class Renderer* renderer;

	ProjectionType projectionType;
	float fov;
	float nearPlane;
	float farPlane;

	D3DXMATRIX view_base;
	D3DXMATRIX view;
	D3DXMATRIX proj;

	bool bEditorCamera;

	float acceleration;
	float drag;
	D3DXVECTOR3 movement_speed;
};