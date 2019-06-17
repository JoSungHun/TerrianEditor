#pragma once
#include "Framework.h"

namespace Window
{
	static HINSTANCE Instance;
	static HWND Handle;
	static std::wstring AppName;
	static bool IsFullScreen;
	static std::function<LRESULT(HWND, uint, WPARAM, LPARAM)> InputProc;
	static std::function<LRESULT(HWND, uint, WPARAM, LPARAM)> EditorProc;
	static std::function<void(const uint&, const uint&)> Resize;


	inline LRESULT CALLBACK WndProc
	(
		HWND handle,
		uint message,
		WPARAM wParam,
		LPARAM lParam
	)
	{
		if (InputProc != nullptr)
			InputProc(handle, message, wParam, lParam);

		if (EditorProc != nullptr)
			EditorProc(handle, message, wParam, lParam);

		switch (message)
		{
		case WM_DISPLAYCHANGE:
		case WM_SIZE:
			if (Resize != nullptr && wParam != SIZE_MINIMIZED)
				Resize(lParam & 0xff, (lParam >> 16) & 0xff);
			break;
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(handle, message, wParam, lParam);
		}

		return 0;
	}

	inline void Create
	(
		HINSTANCE instance,
		const std::wstring& appName,
		const uint& width,
		const uint& height,
		const bool& bFullScreen = false
	)
	{
		Instance = instance;
		AppName = appName;
		IsFullScreen = bFullScreen;

		WNDCLASSEX wndClass;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
		wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		wndClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);
		wndClass.hInstance = instance;
		wndClass.lpfnWndProc = static_cast<WNDPROC>(WndProc);
		wndClass.lpszClassName = appName.c_str();
		wndClass.lpszMenuName = nullptr;
		wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndClass.cbSize = sizeof(WNDCLASSEX);

		WORD check = RegisterClassEx(&wndClass);
		assert(check != 0);

		Handle = CreateWindowExW
		(
			WS_EX_APPWINDOW,
			appName.c_str(),
			appName.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<int>(width),
			static_cast<int>(height),
			nullptr,
			nullptr,
			instance,
			nullptr
		);
		assert(Handle != nullptr);
	}

	inline void Show()
	{
		SetForegroundWindow(Handle);
		SetFocus(Handle);
		ShowCursor(TRUE);
		ShowWindow(Handle, SW_SHOWNORMAL);
		UpdateWindow(Handle);
	}

	inline const bool Update()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return msg.message != WM_QUIT;
	}

	inline void Destroy()
	{
		DestroyWindow(Handle);
		UnregisterClass(AppName.c_str(), Instance);
	}

	inline const uint GetWidth()
	{
		RECT rect;
		GetClientRect(Handle, &rect);
		return static_cast<uint>(rect.right - rect.left);
	}

	inline const uint GetHeight()
	{
		RECT rect;
		GetClientRect(Handle, &rect);
		return static_cast<uint>(rect.bottom - rect.top);
	}
}