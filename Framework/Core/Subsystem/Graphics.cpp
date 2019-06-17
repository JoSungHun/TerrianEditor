#include "Framework.h"
#include "Graphics.h"

Graphics::Graphics(Context * context)
	: ISubsystem(context)
	, device(nullptr)
	, deviceContext(nullptr)
	, swapChain(nullptr)
	, debugDevice(nullptr)
	, gpuMemorySize(0)
	, gpuDescription(L"")
	, numerator(0)
	, denominator(1)
	, rtv(nullptr)
	, dsv(nullptr)
	, clearColor(0xFF555566)
{
}

Graphics::~Graphics()
{
	SAFE_RELEASE(dsv);
	SAFE_RELEASE(rtv);
	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
}

const bool Graphics::Initialize()
{
	IDXGIFactory* factory = nullptr;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&factory);
	assert(SUCCEEDED(hr));

	IDXGIAdapter* adapter = nullptr;
	hr = factory->EnumAdapters(0, &adapter);
	assert(SUCCEEDED(hr));

	IDXGIOutput* adapterOutput;
	hr = adapter->EnumOutputs(0, &adapterOutput);
	assert(SUCCEEDED(hr));

	uint modeCount;
	hr = adapterOutput->GetDisplayModeList
	(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED,
		&modeCount,
		nullptr
	);
	assert(SUCCEEDED(hr));

	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[modeCount];
	hr = adapterOutput->GetDisplayModeList
	(
		DXGI_FORMAT_R8G8B8A8_UNORM
		, DXGI_ENUM_MODES_INTERLACED
		, &modeCount
		, displayModeList
	);
	assert(SUCCEEDED(hr));

	for (uint i = 0; i < modeCount; i++)
	{
		bool isCheck = true;
		isCheck &= displayModeList[i].Width == Settings::Get().GetWidth();
		isCheck &= displayModeList[i].Height == Settings::Get().GetHeight();

		if (isCheck == true)
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
		}
	}

	DXGI_ADAPTER_DESC adapterDesc;
	ZeroMemory(&adapterDesc, sizeof(DXGI_ADAPTER_DESC));

	hr = adapter->GetDesc(&adapterDesc);
	assert(SUCCEEDED(hr));

	gpuMemorySize = adapterDesc.DedicatedVideoMemory / 1024 / 1024;
	gpuDescription = adapterDesc.Description;

	SAFE_DELETE_ARRAY(displayModeList);
	SAFE_RELEASE(adapterOutput);
	SAFE_RELEASE(adapter);
	SAFE_RELEASE(factory);

	CreateSwapChain();
	Resize
	(
		static_cast<uint>(Settings::Get().GetWidth()),
		static_cast<uint>(Settings::Get().GetHeight())
	);
	return true;
}

void Graphics::Resize(const uint & width, const uint & height)
{
	DeleteBackBuffer();
	{
		HRESULT hr = swapChain->ResizeBuffers
		(
			0,
			width,
			height,
			DXGI_FORMAT_UNKNOWN,
			0
		);
		assert(SUCCEEDED(hr));
	}
	CreateRenderTargetView();
	CreateDepthStencilView(width, height);
	SetViewport(width, height);
}

void Graphics::SetViewport(const uint & width, const uint & height)
{
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void Graphics::BeginScene()
{
	deviceContext->RSSetViewports(1, &viewport);
	deviceContext->OMSetRenderTargets(1, &rtv, dsv);
	deviceContext->ClearRenderTargetView(rtv, clearColor);
	deviceContext->ClearDepthStencilView
	(
		dsv,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0
	);
}

void Graphics::EndScene()
{
	HRESULT hr = swapChain->Present(Settings::Get().IsVsync() ? 1 : 0, 0);
	assert(SUCCEEDED(hr));
}

void Graphics::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = Settings::Get().IsVsync() ? numerator : 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = Settings::Get().IsVsync() ? denominator : 1;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.OutputWindow = Settings::Get().GetWindowHandle();
	swapChainDesc.Windowed = !Settings::Get().IsFullScreen();

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		//D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	HRESULT hr = D3D11CreateDeviceAndSwapChain
	(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		featureLevels,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain,
		&device,
		nullptr,
		&deviceContext
	);
	assert(SUCCEEDED(hr));
}

void Graphics::CreateRenderTargetView()
{
	ID3D11Texture2D* backBuffer = nullptr;
	auto hr = swapChain->GetBuffer
	(
		0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&backBuffer)
	);
	assert(SUCCEEDED(hr));

	hr = device->CreateRenderTargetView
	(
		backBuffer,
		nullptr,
		&rtv
	);
	assert(SUCCEEDED(hr));

	SAFE_RELEASE(backBuffer);
}

void Graphics::CreateDepthStencilView(const uint & width, const uint & height)
{
	ID3D11Texture2D* depthStencilBuffer = nullptr;

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	auto hr = device->CreateTexture2D
	(
		&texDesc,
		nullptr,
		&depthStencilBuffer
	);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Format = texDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView
	(
		depthStencilBuffer,
		&dsvDesc,
		&dsv
	);
	assert(SUCCEEDED(hr));

	SAFE_RELEASE(depthStencilBuffer);
}

void Graphics::DeleteBackBuffer()
{
	SAFE_RELEASE(dsv);
	SAFE_RELEASE(rtv);
}