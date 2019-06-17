#include "Framework.h"
#include "Renderable.h"
#include "Transform.h"

Renderable::Renderable(Context * context, Actor * actor, Transform * transform)
	: IComponent(context, actor, transform)
	, material(nullptr)
	, mesh(nullptr)
	, renderable_type(RenderableType::Opaque)
{
	resourceMgr = context->GetSubsystem<ResourceManager>();
}

Renderable::~Renderable()
{
}

void Renderable::Serialize(FileStream * stream)
{
	//Mesh
	stream->Write(mesh ? mesh->GetResourcePath() : NOT_ASSIGNED_STR);

	//Material
	stream->Write(material ? material->GetResourcePath() : NOT_ASSIGNED_STR);
}

void Renderable::Deserialize(FileStream * stream)
{
	//TODO : 
}

void Renderable::OnInitialize()
{
}

void Renderable::OnStart()
{
}

void Renderable::OnUpdate()
{
}

void Renderable::OnStop()
{
}

void Renderable::OnDestroy()
{
}

void Renderable::SetMaterial(const std::string & path)
{
	material = nullptr;
	material = resourceMgr->Load<Material>(path);
}

void Renderable::SetStandardMaterial()
{
	if (auto mat = resourceMgr->GetResourceFromName<Material>("Standard"))
	{
		material = nullptr;
		material = mat;
		return;
	}

	material = nullptr;
	material = new Material(context);
	material->SetColorAlbedo({ 0.6f, 0.6f, 0.6f, 1.0f });
	material->SetStandardShader();
	material->SetResourceName("Standard");

	resourceMgr->RegisterResource(material);
}

void Renderable::CreateEmptyMesh()
{
	mesh = new Mesh(context);
}


void Renderable::SetMesh(const std::string & path)
{
	mesh = nullptr;
	mesh = resourceMgr->Load<Mesh>(path);
}

void Renderable::SetStandardMesh(const MeshType & type)
{
	mesh = nullptr;
	mesh = new Mesh(context);
	mesh->SetMeshType(type);
	mesh->CreateBuffers();

	resourceMgr->RegisterResource(mesh);
}

