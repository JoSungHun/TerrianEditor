#pragma once
#include "stdafx.h"

class IWidget
{
public:
	IWidget(class Context* context)
		: context(context)
		, title("")
		, xMin(0.0f)
		, xMax(std::numeric_limits<float>::max())
		, yMin(0.0f)
		, yMax(std::numeric_limits<float>::max())
		, height(0.0f)
		, windowFlags(ImGuiWindowFlags_NoCollapse)
		, bVisible(true)
	{}
	virtual ~IWidget() = default;

	virtual void Begin()
	{
		if (!bVisible)
			return;

		ImGui::SetNextWindowSize(ImVec2(xMin, yMin), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints
		(
			ImVec2(xMin, yMin),
			ImVec2(xMax, yMax)
		);

		ImGui::Begin(title.c_str(), &bVisible, windowFlags);
	}

	virtual void Render() = 0;

	virtual void End()
	{
		if (!bVisible)
			return;

		height = ImGui::GetWindowHeight();
		ImGui::End();
	}

	const float& GetHeight() const { return height; }
	const bool& IsVisible() const { return bVisible; }
	void SetIsVisible(const bool& bVisible) { this->bVisible = bVisible; }

protected:
	class Context* context;
	std::string title;
	float xMin;
	float xMax;
	float yMin;
	float yMax;
	float height;
	int windowFlags;
	bool bVisible;
};