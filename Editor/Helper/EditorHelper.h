#pragma once

class EditorHelper final
{
public:
	static EditorHelper& Get()
	{
		static EditorHelper instance;
		return instance;
	}

	void Initialize(class Context* context);

	auto GetSelectActor() const -> class Actor* { return selectActor; }
	void SetSelectActor(class Actor* actor) { this->selectActor = actor; }

	//void LoadModel(const std::string& path) const;

private:
	class Context* context;
	class ResourceManager* resourceManager;
	class SceneManager* sceneManager;
	class Thread* thread;
	class Renderer* renderer;
	class Actor* selectActor;
};

static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
{
	return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)
{
	return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}