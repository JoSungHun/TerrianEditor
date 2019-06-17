#pragma once
#include "Framework.h"

class Settings
{
public:
	static Settings& Get()
	{
		static Settings instance;
		return instance;
	}

	auto GetWindowInstance() const -> HINSTANCE { return hInstance; }
	auto GetWindowHandle() const -> HWND { return handle; }
	auto GetWidth() const -> const float& { return width; }
	auto GetHeight() const -> const float& { return height; }
	auto IsVsync() const -> const bool& { return bVsync; }
	auto IsFullScreen() const -> const bool& { return bFullScreen; }
	auto IsReverseZ() const -> const bool& { return bReversZ; }

	void SetWindowInstance(HINSTANCE hInstance) { this->hInstance = hInstance; }
	void SetWindowHandle(HWND handle) { this->handle = handle; }
	void SetWidth(const float& width) { this->width = width; }
	void SetHeight(const float& height) { this->height = height; }
	void SetIsVsync(const bool& bVsync) { this->bVsync = bVsync; }
	void SetIsFullScreen(const bool& bFullScreen) { this->bFullScreen = bFullScreen; }

private:
	Settings() = default;
	~Settings() = default;

private:
	HINSTANCE hInstance = nullptr;
	HWND handle = nullptr;
	float width = 0.0f;
	float height = 0.0f;
	bool bVsync = true;
	bool bFullScreen = false;
	bool bReversZ = true;
};