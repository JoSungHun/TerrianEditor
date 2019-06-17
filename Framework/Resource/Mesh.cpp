#include "Framework.h"
#include "Mesh.h"

Mesh::Mesh(Context * context)
	: IResource(context)
	, topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	, meshType(MeshType::Custom)
{
	vertexBuffer = new VertexBuffer(context);
	indexBuffer = new IndexBuffer(context);
}

Mesh::~Mesh()
{
	SAFE_DELETE(indexBuffer);
	SAFE_DELETE(vertexBuffer);
}

void Mesh::SaveToFile(const std::string & path)
{
}

void Mesh::LoadFromFile(const std::string & path)
{
}

void Mesh::SetMeshType(const MeshType & type)
{
	if (meshType == type)
		return;

	meshType = type;

	Clear();
	vertexBuffer->Clear();
	indexBuffer->Clear();
}

void Mesh::CreateBuffers()
{
	switch (meshType)
	{
	case MeshType::Custom:
		break;
	case MeshType::Cube:
		SetResourceName("Cube");
		break;
	case MeshType::Cylinder:
		SetResourceName("Cylinder");
		break;
	case MeshType::Cone:
		SetResourceName("Cone");
		break;
	case MeshType::Capsule:
		SetResourceName("Capsule");
		break;
	case MeshType::Sphere:
		GeometryUtility::CreateSphere(*this);
		SetResourceName("Sphere");
		break;
	case MeshType::Quad:
		//GeometryUtility::CreateQuad(geometry);
		SetResourceName("Quad");
		break;
	case MeshType::ScreenQuad:
		SetResourceName("ScreenQuad");
		break;
	default:
		break;
	}

	vertexBuffer->Create(vertices);
	indexBuffer->Create(indices);
}
