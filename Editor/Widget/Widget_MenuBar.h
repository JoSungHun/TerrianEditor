#pragma once
#include "IWidget.h"

class Widget_MenuBar final : public IWidget
{
public:
	Widget_MenuBar(class Context* context);
	~Widget_MenuBar() = default;

	void Begin() override {}
	void Render() override;
	void End() override {}

private:

private:
	bool bShowMetricsWindow;
	bool bShowStyleEditor;
	bool bShowDemoWindow;

	//save load
	bool bShowSave;
	bool bShowLoad;
};