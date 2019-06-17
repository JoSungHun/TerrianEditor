#pragma once
#include "IComponent.h"

class Renderable final : public IComponent
{
public:
	Renderable
	(
		class Context* context,
		class Actor* actor,
		class Transform* transform
	);
	~Renderable();

	Renderable(const Renderable&) = delete;
	Renderable& operator=(const Renderable&) = delete;

	void Serialize(FileStream* stream) override;
	void Deserialize(FileStream* stream) override;

	void OnInitialize() override;
	void OnStart() override;
	void OnUpdate() override;
	void OnStop() override;
	void OnDestroy() override;

	auto GetMaterial()->class Material* const { return material; }
	void SetMaterial(class Material* material) { this->material = material; }
	void SetMaterial(const std::string& path);
	void SetStandardMaterial();

	void CreateEmptyMesh();
	auto GetMesh()->class Mesh* const { return mesh; }
	void SetMesh(class Mesh* mesh) { this->mesh = mesh; }
	void SetMesh(const std::string& path);
	void SetStandardMesh(const MeshType& type);

	auto IsCastShadow() const -> const bool& { return bCast_shadow; }
	void SetIsCastShadow(const bool& bCast_shadow) { this->bCast_shadow = bCast_shadow; }

	auto IsReceiveShadow() const -> const bool& { return bReceive_shadow; }
	void SetIsReceiveShadow(const bool& bReceive_shadow) { this->bReceive_shadow = bReceive_shadow; }

	auto GetRenderableType() const -> const RenderableType& { return renderable_type; }
	void SetRenderableType(const RenderableType& type) { this->renderable_type = type; }

private:
	class ResourceManager* resourceMgr;
	class Material* material;
	class Mesh* mesh;

	RenderableType renderable_type;

	bool bCast_shadow;
	bool bReceive_shadow;
};