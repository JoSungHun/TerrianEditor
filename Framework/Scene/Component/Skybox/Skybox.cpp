#include "Framework.h"
#include "Skybox.h"
#include "../Renderable.h"
#include "Scene/Actor.h"

Skybox::Skybox(Context * context, Actor * actor, Transform * transform)
	: IComponent(context, actor, transform)
{
	skybox_type = SkyboxType::Sphere;

	material = std::make_shared<Material>(context);
	material->SetCullMode(CullMode::FRONT);
	material->SetColorAlbedo(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	material->SetShadingMode(ShadingMode::Sky);

	switch (skybox_type)
	{
	case SkyboxType::Cube:
		//TODO :
		break;
	case SkyboxType::Sphere:
		paths = { "../_Assets/Cubemap/sky_sphere.hdr" };
		break;
	}
}

void Skybox::Serialize(FileStream * stream)
{
}

void Skybox::Deserialize(FileStream * stream)
{
}

void Skybox::OnInitialize()
{
	context->GetSubsystem<Thread>()->AddTask([this]()
	{
		switch (skybox_type)
		{
		case SkyboxType::Cube:      CreateFromCube(paths);           break;
		case SkyboxType::Sphere:    CreateFromSphere(paths.front()); break;
		}
	});
}

void Skybox::OnStart()
{
}

void Skybox::OnUpdate()
{
}

void Skybox::OnStop()
{
}

void Skybox::OnDestroy()
{
}

void Skybox::CreateFromCube(const std::vector<std::string>& paths)
{
}

void Skybox::CreateFromSphere(const std::string & path)
{
	LOG_INFO("Creating HDR sky sphere...");

	//Texture
	{
		bool bGenerate_mipmap = true;
		texture = std::make_shared<Texture>(context);
		texture->SetIsNeedsMipChain(bGenerate_mipmap);
		texture->LoadFromFile(path);
		texture->SetResourceName("SkySphere");
	}

	//Material
	{
		material->SetResourceName("Standard_SkySphere");
		material->SetTexture(TextureType::Albedo, texture.get());
	}

	//Renderable
	{
		auto renderable = actor->AddComponent<Renderable>();
		renderable->SetStandardMesh(MeshType::Sphere);
		renderable->SetMaterial(material.get());
	}

	transform->SetScale(D3DXVECTOR3(980, 980, 980));

	LOG_INFO("Sky sphere has been created successfully");
}