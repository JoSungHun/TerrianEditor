#pragma once
#include "ISubsystem.h"

class Graphics final : public ISubsystem
{
public:
	Graphics(class Context* context);
	~Graphics();

	const bool Initialize() override;

	ID3D11Device* GetDevice() const { return device; }
	ID3D11DeviceContext* GetDeviceContext() const { return deviceContext; }
	IDXGISwapChain* GetSwapChain() const { return swapChain; }

	void Resize(const uint& width, const uint& height);
	void SetViewport(const uint& width, const uint& height);

	void BeginScene();
	void EndScene();

private:
	void CreateSwapChain();
	void CreateRenderTargetView();
	void CreateDepthStencilView(const uint& width, const uint& height);

	void DeleteBackBuffer();


private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	IDXGISwapChain* swapChain;

	ID3D11Debug* debugDevice;
	uint gpuMemorySize;
	std::wstring gpuDescription;
	uint numerator;
	uint denominator;

	ID3D11RenderTargetView* rtv;
	ID3D11DepthStencilView* dsv;
	D3D11_VIEWPORT viewport;
	D3DXCOLOR clearColor;
};