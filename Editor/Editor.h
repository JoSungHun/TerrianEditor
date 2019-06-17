#pragma once

class Editor final
{
public:
	Editor();
	~Editor();

	Editor(const Editor&) = delete;
	Editor& operator=(const Editor&) = delete;

	void Resize(const uint& width, const uint& height);
	void Update();
	void Render();

private:
	void BeginDockspace();
	void EndDockspace();

	void ApplyStyle();

private:
	class Context* context;
	class Engine* engine;
	class Graphics* graphics;
	bool bUseDockspace;

	std::vector<class IWidget*> widgets;
};