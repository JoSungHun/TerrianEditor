#pragma once
#include "IWidget.h"

class Widget_Scene final : public IWidget
{
public:
	Widget_Scene(class Context* context);
	~Widget_Scene() = default;

	Widget_Scene(const Widget_Scene&) = delete;
	Widget_Scene& operator=(const Widget_Scene&) = delete;

	void Render() override;

private:
	void ShowFrame();
	void ShowTransformGizmo();

private:
	class Timer* timer;
	class Renderer* renderer;

	ImVec2 framePos;
	ImVec2 frameSize;
	float frameTimer;
};