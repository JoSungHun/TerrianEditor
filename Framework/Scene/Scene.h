#pragma once
//class Scene final
//{
//	friend class Widget_Editor;
//public:
//	Scene(class Context* context);
//	~Scene();
//
//	class Camera* GetCamera() const { return camera; }
//
//	void Update();
//	void Render();
//
//	void CreateTerrain(const std::string& filePath);
//	void CreateTerrain(const uint& width, const uint& height);
//	
//private:
//	class Context* context;
//	class Graphics* graphics;
//	
//	bool controlMain;
//	class Camera* camera;
//	class SubCamera* subCamera;
//	class ConstantBuffer* cameraBuffer;
//
//
//	class RenderTexture* shadowTexture[MAX_SHADOW_LEVEL];
//	class RenderTexture* reflectionTexture;
//	class RenderTexture* refractionTexture;
//
//
//	class Terrain* terrain; 
//
//	bool oceanRender;
//	class Ocean* ocean;
//
//	class Light* sunLight;
//
//};
#pragma once

class Scene final
{
public:
	Scene(class Context* context);
	~Scene();

	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	void SaveToFile(const std::string& path);
	void LoadFromFile(const std::string& path);

	auto GetName() const -> const std::string& { return name; }
	void SetName(const std::string& name) { this->name = name; }

	auto GetActors() const -> const std::vector<class Actor*>& { return actors; }
	auto GetActorFromID(const uint& id) const -> class Actor*;

	auto CreateActor()-> class Actor*;
	void AddActor(class Actor* actor);
	void RemoveActor(class Actor* actor);
	auto IsExistActor(class Actor* actor)->const bool;

	void Update();

private:
	auto CreateSkybox() -> class Actor*;
	auto CreateCamera() -> class Actor*;
	auto CreateDirectionalLight() -> class Actor*;
	auto CreateTerrain() -> class Actor*;
	auto CreateOcean() -> class Actor*;

private:
	class Context* context;
	class Renderer* renderer;
	std::vector<class Actor*> actors;

	//Short cut to main Actors
	class Actor* camera;
	class Actor* terrain;
	class Actor* light;
	class Actor* skybox;
	class Actor* ocean;

	std::string name;
	bool bEditorMode;
	bool bAdded;
};