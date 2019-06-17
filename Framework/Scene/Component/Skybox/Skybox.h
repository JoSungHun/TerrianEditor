#pragma once
#include "../IComponent.h"

enum class SkyboxType : uint
{
	Cube,
	Sphere,
};

class Skybox final : public IComponent
{
public:
	Skybox
	(
		class Context* context,
		class Actor* actor,
		class Transform* transform
	);
	~Skybox() = default;

	Skybox(const Skybox&) = delete;
	Skybox& operator=(const Skybox&) = delete;

	void Serialize(FileStream* stream) override;
	void Deserialize(FileStream* stream) override;

	void OnInitialize() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;
	void OnDestroy() override;

	auto GetTexture() const -> class Texture* { return texture.get(); }
	auto GetMaterial() const -> class Material* { return material.get(); }

private:
	void CreateFromCube(const std::vector<std::string>& paths);
	void CreateFromSphere(const std::string& path);

private:
	SkyboxType skybox_type;
	std::vector<std::string> paths;
	std::shared_ptr<class Texture> texture;
	std::shared_ptr<class Material> material;
};