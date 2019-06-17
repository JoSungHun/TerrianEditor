#include "Framework.h"
#include "SceneManager.h"
#include "../../Scene/Scene.h"

SceneManager::SceneManager(Context * context)
	: ISubsystem(context)
	, currentScene(nullptr)
{
	EventSystem::Get().Subscribe(EventType::Update, EVENT_HANDLER(Update));
}

SceneManager::~SceneManager()
{
	//for (auto iter = scenes.begin(); iter != scenes.end(); iter++)
	//	SAFE_DELETE(iter->second);

	for (auto scene : scenes)
		SAFE_DELETE(scene.second);
}

const bool SceneManager::Initialize()
{
	RegisterScene("First", new Scene(context));
	SetCurrentScene("First");

	return true;
}

void SceneManager::SetCurrentScene(const std::string & sceneName)
{
	assert(scenes.count(sceneName) > 0);
	currentScene = scenes[sceneName];
}

void SceneManager::RegisterScene(const std::string & sceneName, Scene * scene)
{
	assert(scenes.count(sceneName) < 1);
	scene->SetName(sceneName);
	scenes.insert(std::make_pair(sceneName, scene));
}

void SceneManager::Update()
{
	if (currentScene)
		currentScene->Update();
}