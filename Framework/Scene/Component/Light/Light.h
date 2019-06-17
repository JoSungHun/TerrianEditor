#pragma once
#include "../IComponent.h"

enum class LightType : uint
{
	Directional,
	Point,
	Spot,
};

class Light final : public IComponent
{
public:
	Light
	(
		class Context* context,
		class Actor* actor,
		class Transform* transform
	);
	~Light();

	Light(const Light& rhs) = delete;
	Light& operator=(const Light& rhs) = delete;

	void Serialize(FileStream* stream) override;
	void Deserialize(FileStream* stream) override;

	void OnInitialize() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;
	void OnDestroy() override;

	auto GetDirection() const->D3DXVECTOR3;

	auto GetLightType() const -> const LightType& { return light_type; }
	void SetLightType(const LightType& type);

	auto GetRange() const -> const float& { return range; }
	void SetRange(const float& range);

	auto GetIntensity() const -> const float& { return intensity; }
	void SetIntensity(const float& intensity) { this->intensity = intensity; }

	auto GetAngle() const -> const float& { return radian_angle; }
	void SetAngle(const float& angle);

	auto GetBias() const -> const float& { return bias; }
	void SetBias(const float& bias) { this->bias = bias; }

	auto GetNormalBias() const -> const float& { return normal_bias; }
	void SetNormalBias(const float& normal_bias) { this->normal_bias = normal_bias; }

	auto GetColor() const -> const D3DXCOLOR& { return color; }
	void SetColor(const D3DXCOLOR& color) { this->color = color; }

	auto IsCastShadow() const -> const bool& { return bCast_shadow; }
	void SetIsCastShadow(const bool& bCast_shadow);

	auto GetViewMatrix(const uint& index = 0) -> const D3DXMATRIX;
	auto GetProjectionMatrix(const uint& index = 0) -> const D3DXMATRIX;
	auto GetShadowMap() const -> RenderTexture* { return shadow_map.get(); }

private:
	void ClampRotation();
	void ComputeViewMatrix();
	auto ComputeProjectionMatrix(const uint& index = 0) -> const bool;
	void CreateShadowMap(const bool& force);

private:
	Renderer* renderer;
	LightType light_type;
	float range;
	float intensity;
	float radian_angle;
	float bias;
	float normal_bias;
	D3DXCOLOR color;
	D3DXVECTOR3 last_camera_position;
	D3DXVECTOR3 last_light_position;
	D3DXQUATERNION last_light_rotation;

	bool bCast_shadow;
	bool bUpdate;

	std::vector<D3DXMATRIX> views;
	std::vector<D3DXMATRIX> projections;
	std::shared_ptr<RenderTexture> shadow_map;
};