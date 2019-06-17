#include "stdafx.h"
#include "Editor.h"
#include "Widget_MenuBar.h"
//#include "Widget_Editor.h"


Widget_MenuBar::Widget_MenuBar(Context * context)
	: IWidget(context)
	, bShowMetricsWindow(false)
	, bShowStyleEditor(false)
	, bShowDemoWindow(false)
{

}

void Widget_MenuBar::Render()
{
	if (ImGui::BeginMenuBar())
	{
		//Tools
		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Metrics", nullptr, &bShowMetricsWindow);
			ImGui::MenuItem("Style", nullptr, &bShowStyleEditor);
			ImGui::MenuItem("Demo", nullptr, &bShowDemoWindow);
			
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Show"))
		{
			
			ImGui::EndMenu();
		}
		 

		ImGui::EndMenuBar();
	}

	auto StyleEditor = []()
	{
		ImGui::Begin("StyleEditor");
		ImGui::ShowStyleEditor();
		ImGui::End();
	};

	if (bShowMetricsWindow) ImGui::ShowMetricsWindow();
	if (bShowStyleEditor)	StyleEditor();
	if (bShowDemoWindow)	ImGui::ShowDemoWindow();
}
