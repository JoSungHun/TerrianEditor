#include "Framework.h"
#include "Scene.h"
#include "Actor.h"
#include "./Component/Camera.h"
#include "./Component/Skybox/Skybox.h"
#include "./Component/Renderable.h"
#include "./Component/Light/Light.h"
#include "./Component/Terrain/Terrain.h"
#include "./Component/Ocean/Ocean.h"

Scene::Scene(Context * context)
	: context(context)
	, name("")
	, bEditorMode(true)
	, bAdded(false)
{
	renderer = context->GetSubsystem<Renderer>();

	skybox = CreateSkybox();
	camera = CreateCamera();
	light = CreateDirectionalLight();

	//TEST
	terrain = CreateTerrain();
	ocean = CreateOcean();


	terrain->GetComponent<Terrain>()->CreateFromHeightMap("../_Assets/HM.bmp");
	ocean->GetComponent<Ocean>()->EditOceanSize(terrain->GetComponent<Terrain>()->GetTerrainWidth(), terrain->GetComponent<Terrain>()->GetTerrainHeight());
}

Scene::~Scene()
{
	for (auto& actor : actors)
		SAFE_DELETE(actor);

	actors.clear();
	actors.shrink_to_fit();
}

void Scene::SaveToFile(const std::string & path)
{
}

void Scene::LoadFromFile(const std::string & path)
{
}

auto Scene::GetActorFromID(const uint & id) const -> class Actor *
{
	for (const auto& actor : actors)
	{
		if (actor->GetID() == id)
			return actor;
	}
	return nullptr;
}

auto Scene::CreateActor() -> class Actor *
{
	auto actor = new Actor(context);
	actor->Initialize(actor->AddComponent<Transform>());
	AddActor(actor);

	return actor;
}

void Scene::AddActor(Actor * actor)
{
	actors.emplace_back(actor);
	bAdded = true;
}

void Scene::RemoveActor(Actor * actor)
{
}

auto Scene::IsExistActor(Actor * actor) -> const bool
{
	return false;
}

void Scene::Update()
{
	bool bStarted = Engine::IsOnEngineFlag(EngineFlags_Game) && bEditorMode;
	bool bStopped = !Engine::IsOnEngineFlag(EngineFlags_Game) && !bEditorMode;
	bEditorMode = !Engine::IsOnEngineFlag(EngineFlags_Game);

	if (bAdded)
	{
		renderer->AcquireRenderables(this);
		bAdded = false;
	}

	if (bStarted)
	{
		for (auto& actor : actors)
			actor->Start();
	}

	if (bStopped)
	{
		for (auto& actor : actors)
			actor->Stop();
	}

	for (auto& actor : actors)
		actor->Update();
}

auto Scene::CreateSkybox() -> class Actor *
{
	auto skybox = CreateActor();
	skybox->SetName("Skybox");
	skybox->AddComponent<Skybox>();

	return skybox;
}

auto Scene::CreateCamera() -> class Actor *
{
	auto camera = CreateActor();
	camera->SetName("SceneCamera");
	camera->AddComponent<Camera>();

	return camera;
}

auto Scene::CreateDirectionalLight() -> class Actor *
{
	auto light = CreateActor();
	light->SetName("DirectionalLight");
	light->GetTransform()->SetRotationFromEulerAngle(D3DXVECTOR3(30.0f, 0.0f, 0.0f));
	light->GetTransform()->SetTranslation(D3DXVECTOR3(0.0f, 10.0f, 0.0f));

	auto component = light->AddComponent<Light>();
	component->SetLightType(LightType::Directional);
	component->SetIntensity(1.5f);

	return light;
}

auto Scene::CreateTerrain() -> class Actor *
{
	auto terrain = CreateActor();
	terrain->SetName("terrain");
	terrain->AddComponent<Terrain>();
	bAdded = true;

	return terrain;
}

auto Scene::CreateOcean() -> class Actor *
{
	auto ocean = CreateActor();
	ocean->SetName("ocean");
	ocean->AddComponent<Ocean>();
	bAdded = true;

	return ocean;
}




#pragma region OldVersion

//#include "framework.h"
//#include "Scene.h"
//#include "./Component/Camera.h"
//#include "./Component/SubCamera.h"
//#include "./Component/Terrain/Terrain.h"
//#include "./Component/Vegitation/Grass.h"
//#include "./Component/Ocean/Ocean.h"
//
//#define GrassTest
//
//Scene::Scene(Context * context)
//	: context(context)
//	, camera(nullptr)
//	, terrain(nullptr)
//	, sunLight(nullptr)
//	, ocean(nullptr)
//{
//	
//	graphics = context->GetSubsystem<Graphics>();
//
//	//camera
//	{
//		controlMain = true;
//		camera = new Camera(context);
//		subCamera = new SubCamera(context);
//		//subCamera = new SubCamera(context);
//		cameraBuffer = new ConstantBuffer(context);
//		cameraBuffer->Create<CameraData>();
//	}
//
//
//	//Light
//	{
//		sunLight = new Light();
//		sunLight->SetType(LightType::LIGHT_DIRECTIONAL);
//		sunLight->SetScreen(camera->GetNearPlane(), camera->GetFarPlane());
//		sunLight->SetOrthoWidths(100.0f);
//		sunLight->SetPosition(D3DXVECTOR3(-30.0f, 0.f, 30.0f));
//		sunLight->SetDiffuseColor(D3DXCOLOR(1, 1, 1, 1));
//		sunLight->SetAmbientColor(D3DXCOLOR(0.15f, 0.15f, 0.15f, 1.0f));
//		
//	}
//	
//	// Set Render Texture
//	{
//		for (int i = 0; i < MAX_SHADOW_LEVEL; i++)
//		{
//			shadowTexture[i] = new RenderTexture(context);
//			shadowTexture[i]->Create(Settings::Get().GetWidth(), Settings::Get().GetHeight(), camera->GetFarPlane(), camera->GetNearPlane());
//		}
//		reflectionTexture = new RenderTexture(context);
//		reflectionTexture->Create(Settings::Get().GetWidth(), Settings::Get().GetHeight(), camera->GetFarPlane(), camera->GetNearPlane());
//		refractionTexture = new RenderTexture(context);
//		refractionTexture->Create(Settings::Get().GetWidth(), Settings::Get().GetHeight(), camera->GetFarPlane(), camera->GetNearPlane());
//	}
//	
//	//ocean
//	{
//		ocean = new Ocean(context);
//		ocean->SetPosY(5.0f);
//		ocean->CreateOcean(100, 100);
//		ocean->SetReflectionTexture(reflectionTexture);
//		ocean->SetRefractionTexture(refractionTexture);
//		oceanRender = false;
//
//	}
//}
//
//Scene::~Scene()
//{
//}
//
//void Scene::Update()
//{
//	auto input = context->GetSubsystem<Input>();
//
//	//Input Setting
//	{
//		if (input->KeyDown(VK_F3))
//		{
//			if (!controlMain)
//			{
//				controlMain = true;
//			}
//			else
//			{
//				controlMain = false;
//			}
//		}
//
//		if (input->KeyDown(VK_F6))
//		{
//			if (!oceanRender)
//			{
//				oceanRender = true;
//			}
//			else
//			{
//				oceanRender = false;
//			}
//		}
//
//		/*if (input->KeyDown(VK_SPACE))
//		{
//			sunLight->SetPosition(camera->GetPosition());
//		}*/
//
//		if (input->KeyDown(VK_F5))
//		{
//
//			for (int i = 0; i < MAX_SHADOW_LEVEL; i++)
//			{
//				std::string filePath = "../_Assets/Saves/ShadowMap";
//				char c[128];
//				itoa(i, c, 10);
//				filePath.append(c);
//				filePath.append(".bmp");
//
//				shadowTexture[i]->SaveTextureToFile(filePath);
//			}
//
//
//			reflectionTexture->SaveTextureToFile("../_Assets/Saves/reflection.bmp");
//			refractionTexture->SaveTextureToFile("../_Assets/Saves/refraction.bmp");
//		}
//
//		if (input->KeyDown(VK_F7))
//		{
//			camera->ChangeAngleToReflection(ocean->GetPosY());
//		}
//
//		if (input->KeyDown(VK_F11))
//		{
//			terrain->ShaderReload();
//			ocean->ShaderReload();
//		}
//	}
//
//	//Updates
//	{
//		if (controlMain)
//			camera->Update();
//		else
//			subCamera->Update();
//
//		if (sunLight)
//			sunLight->Update();
//
//		if (terrain)
//			terrain->Update();
//
//		if (ocean)
//			ocean->Update();
//
//	}
//	
//
//
//	
//
//	// SubCamera
//	auto cameraData = static_cast<CameraData*>(cameraBuffer->Map());
//	{
//		D3DXMatrixTranspose(&cameraData->View, &camera->GetViewMatrix());
//		D3DXMatrixTranspose(&cameraData->Proj, &camera->GetProjectionMatrix());
//	}
//	cameraBuffer->Unmap();
//}
//
//void Scene::Render()
//{
//	//RenderScene for Shadowmap
//	// Shadow Map
//	{
//		CameraData lightCameraData;
//		sunLight->GenerateProjectionMatrix();
//		sunLight->GenerateViewMatrix();
//		sunLight->GenerateOrthoMatrix();
//		
//		// Light Class 안에서 하도록
//		for (int i = 0; i < MAX_SHADOW_LEVEL; i++)
//		{
//			shadowTexture[i]->SetRenderTarget();
//			shadowTexture[i]->ClearRenderTarget(D3DXCOLOR(0.f, 0.f, 0.f, 0.f));
//
//			sunLight->GetCameraData(lightCameraData, i);
//			if (terrain)
//			{
//				terrain->SetRenderTexture(shadowTexture[i]);
//				terrain->SetLight(sunLight);
//				terrain->DepthRender(lightCameraData);
//			}
//		}
//
//	} 
//
//	reflectionTexture->SetRenderTarget();
//	reflectionTexture->ClearRenderTarget(D3DXCOLOR(0.f, 0.f, 0.f, 0.f));
//	//Refleaction Render
//	{
//		D3DXMATRIX reflectionView = camera->GetReflectionViewMatrix(ocean->GetPosY());
//
//		CameraData reflectionCamera;
//		//Ortho
//		D3DXMatrixTranspose(&reflectionCamera.Proj, &camera->GetProjectionMatrix());
//		//Perspective
//		D3DXMatrixTranspose(&reflectionCamera.Proj, &camera->GetProjectionMatrix());
//		D3DXMatrixTranspose(&reflectionCamera.View, &reflectionView);
//
//		if (terrain)
//		{
//			terrain->Render(reflectionCamera);
//		}
//	}
//
//	refractionTexture->SetRenderTarget();
//	refractionTexture->ClearRenderTarget(D3DXCOLOR(0.f, 0.f, 0.f, 0.f));
//	//Refraction Render
//	{
//		D3DXVECTOR4 clipPlane;
//
//		clipPlane = D3DXVECTOR4(0.0f, -1.0f, 0.0f, ocean->GetPosY());
//
//		CameraData refractionCamera;
//		D3DXMatrixTranspose(&refractionCamera.Proj, &camera->GetProjectionMatrix());
//		D3DXMatrixTranspose(&refractionCamera.View, &camera->GetViewMatrix());
//
//		if (terrain)
//		{
//			terrain->refractionRender(refractionCamera, ClipPlaneData(clipPlane));
//		}
//	}
//
//
//
//	graphics->SetBackBufferRenderTarget();
//	graphics->ResetViewport();
//	// Main Back Buffer Render
//	{
//		CameraData cameraData;
//		D3DXMatrixTranspose(&cameraData.View, &camera->GetViewMatrix());
//		D3DXMatrixTranspose(&cameraData.Proj, &camera->GetProjectionMatrix());
//
//		if (terrain)
//		{
//
//			subCamera->Render(cameraBuffer);
//			terrain->Render(cameraData);
//		}
//
//		if (oceanRender)
//		{
//			D3DXMATRIX reflectionView = camera->GetReflectionViewMatrix(ocean->GetPosY());
//
//			if (ocean)
//				ocean->Render(cameraData, reflectionView);
//		}
//	}
//	
//
//	 camera->DebugRender();
//}
//
//void Scene::CreateTerrain(const std::string & filePath)
//{
//	terrain = new Terrain(context);
//	terrain->SetLight(sunLight);
//	terrain->CreateTerrain(filePath);
//	sunLight->SetPosition(D3DXVECTOR3(static_cast<float>(terrain->GetWidth() / 2), 500, static_cast<float>(terrain->GetHeight() / 2)));
//}
//
//void Scene::CreateTerrain(const uint& width, const uint& height)
//{
//	terrain = new Terrain(context);
//	terrain->SetLight(sunLight);
//	sunLight->SetPosition(D3DXVECTOR3(static_cast<float>(width / 2), 500, static_cast<float>(height / 2)));
//	terrain->CreateTerrain(width, height);
//}


#pragma endregion