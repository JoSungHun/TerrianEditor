#pragma once
#include "ISubsystem.h"

class SceneManager final : public ISubsystem
{
public:
	SceneManager(class Context* context);
	~SceneManager();

	const bool Initialize() override;

	auto GetScenes() const -> const std::map<std::string, class Scene*>& { return scenes; }
	auto GetCurrentScene() const -> class Scene* { return currentScene; }
	void SetCurrentScene(const std::string& sceneName);

	void RegisterScene(const std::string& sceneName, class Scene* scene);

	void Update();

private:
	class Scene* currentScene;
	std::map<std::string, class Scene*> scenes;
};