#pragma once
#include "IResource.h"

enum class MeshType : uint
{
	Custom,
	Cube,
	Cylinder,
	Cone,
	Capsule,
	Sphere,
	Quad,
	ScreenQuad,
};

class Mesh final : public IResource, public Geometry<VertexModel>
{
public:
	Mesh(class Context* context);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	void SaveToFile(const std::string& path) override;
	void LoadFromFile(const std::string& path) override;

	auto GetMeshType() const -> const MeshType& { return meshType; }
	void SetMeshType(const MeshType& type);

	auto GetVertexBuffer()->VertexBuffer* const { return vertexBuffer; }
	auto GetIndexBuffer()->IndexBuffer* const { return indexBuffer; }

	auto GetPrimitiveTopology() const -> const D3D11_PRIMITIVE_TOPOLOGY& { return topology; }
	void SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY& topology) { this->topology = topology; }

	void CreateBuffers();

private:
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	D3D11_PRIMITIVE_TOPOLOGY topology;
	MeshType meshType;
};