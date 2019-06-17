#include "Framework.h"
#include "DirectWrite.h"

DirectWrite::DirectWrite(Context * context)
	: ISubsystem(context)
{
}

DirectWrite::~DirectWrite()
{
	SAFE_RELEASE(targetBitamp);
	SAFE_RELEASE(writeDeviceContext);
	SAFE_RELEASE(writeDevice);
	SAFE_RELEASE(factory);
	SAFE_RELEASE(writeFactory);
}

const bool DirectWrite::Initialize()
{
	auto graphics = context->GetSubsystem<Graphics>();

	//Create Write Factory
	HRESULT hr = DWriteCreateFactory
	(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&writeFactory)
	);
	assert(SUCCEEDED(hr));

	//Create D2D Factory
	D2D1_FACTORY_OPTIONS option;
	option.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &factory);
	assert(SUCCEEDED(hr));

	//Get 3D Information
	IDXGIDevice* dxgiDevice;
	hr = graphics->GetDevice()->QueryInterface(&dxgiDevice);
	assert(SUCCEEDED(hr));

	IDXGISurface* dxgiSurface;
	hr = graphics->GetSwapChain()->GetBuffer
	(
		0,
		__uuidof(IDXGISurface),
		reinterpret_cast<void**>(&dxgiSurface)
	);
	assert(SUCCEEDED(hr));


	//Create Write Device
	hr = factory->CreateDevice(dxgiDevice, &writeDevice);
	assert(SUCCEEDED(hr));

	//Create Write Device Context
	hr = writeDevice->CreateDeviceContext
	(
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
		&writeDeviceContext
	);
	assert(SUCCEEDED(hr));

	//Create D2D BackBuffer
	D2D1_BITMAP_PROPERTIES1 bp;
	bp.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	bp.dpiX = 96;
	bp.dpiY = 96;
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
	bp.colorContext = nullptr;

	hr = writeDeviceContext->CreateBitmapFromDxgiSurface
	(
		dxgiSurface,
		&bp,
		&targetBitamp
	);
	assert(SUCCEEDED(hr));

	writeDeviceContext->SetTarget(targetBitamp);

	return true;
}

void DirectWrite::BeginTextDraw()
{
	writeDeviceContext->BeginDraw();
}

void DirectWrite::EndTextDraw()
{
	HRESULT hr = writeDeviceContext->EndDraw();
	assert(SUCCEEDED(hr));
}

void DirectWrite::Text(const std::wstring & text, const D3DXVECTOR2 & position, const D3DXCOLOR & color, const std::wstring & fontName, const float & fontSize, const DWRITE_FONT_WEIGHT & fontWeight, const DWRITE_FONT_STYLE & fontStyle, const DWRITE_FONT_STRETCH & fontStretch)
{
	D2D1_RECT_F range;
	range.left = position.x;
	range.top = position.y;
	range.right = position.x + text.size() * fontSize;
	range.bottom = position.y + fontSize;

	auto brush = RegistBrush(color);
	auto format = RegistFormat(fontName, fontSize, fontWeight, fontStyle, fontStretch);

	writeDeviceContext->DrawTextW
	(
		text.c_str(),
		text.size(),
		format,
		range,
		brush
	);
}

ID2D1SolidColorBrush * DirectWrite::RegistBrush(const D3DXCOLOR & color)
{
	ID2D1SolidColorBrush* brush = FindBrush(color);

	if (!brush)
	{
		D2D1::ColorF brushColor = D2D1::ColorF(color.r, color.g, color.b);
		HRESULT hr = writeDeviceContext->CreateSolidColorBrush
		(
			brushColor,
			&brush
		);
		assert(SUCCEEDED(hr));

		brushes.push_back(std::make_pair(color, brush));
	}

	return brush;
}

IDWriteTextFormat * DirectWrite::RegistFormat(const std::wstring & fontName, const float & fontSize, const DWRITE_FONT_WEIGHT & fontWeight, const DWRITE_FONT_STYLE & fontStyle, const DWRITE_FONT_STRETCH & fontStretch)
{
	Font font =
	{
		fontName,
		fontSize,
		fontWeight,
		fontStyle,
		fontStretch
	};

	IDWriteTextFormat* format = FindFormat(font);

	if (!format)
	{
		HRESULT hr = writeFactory->CreateTextFormat
		(
			font.Name.c_str(),
			nullptr,
			font.Weight,
			font.Style,
			font.Stretch,
			font.Size,
			L"ko",
			&format
		);
		assert(SUCCEEDED(hr));

		formats.push_back(std::make_pair(font, format));
	}

	return format;
}

ID2D1SolidColorBrush * DirectWrite::FindBrush(const D3DXCOLOR & color)
{
	for (auto& temp : brushes)
	{
		if (temp.first == color)
			return temp.second;
	}
	return nullptr;
}

IDWriteTextFormat * DirectWrite::FindFormat(const Font & font)
{
	for (auto& temp : formats)
	{
		if (temp.first == font)
			return temp.second;
	}
	return nullptr;
}
