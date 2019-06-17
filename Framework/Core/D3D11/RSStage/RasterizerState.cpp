#include "Framework.h"
#include "RasterizerState.h"

std::map<CullMode, D3D11_CULL_MODE> RasterizerState::cull_modes
{
	std::make_pair(CullMode::NONE, D3D11_CULL_NONE),
	std::make_pair(CullMode::FRONT, D3D11_CULL_FRONT),
	std::make_pair(CullMode::BACK, D3D11_CULL_BACK),
};

std::map<FillMode, D3D11_FILL_MODE> RasterizerState::fill_modes
{
	std::make_pair(FillMode::SOLID, D3D11_FILL_SOLID),
	std::make_pair(FillMode::WIREFRAME, D3D11_FILL_WIREFRAME),
};

auto RasterizerState::GetBitMask(const CullMode & cull_mode, const FillMode & fill_mode) -> const uint
{
	return
		static_cast<uint>(cull_mode) |
		static_cast<uint>(fill_mode);
}

RasterizerState::RasterizerState(Context * context)
	: state(nullptr)
{
	graphics = context->GetSubsystem<Graphics>();
}

RasterizerState::~RasterizerState()
{
	Clear();
}

auto RasterizerState::Create(const CullMode & cull_mode, const FillMode & fill_mode) -> const uint
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
	desc.CullMode = cull_modes[cull_mode];
	desc.FillMode = fill_modes[fill_mode];
	desc.DepthClipEnable = true;
	desc.FrontCounterClockwise = false;

	auto result = SUCCEEDED(graphics->GetDevice()->CreateRasterizerState(&desc, &state));
	if (!result)
	{
		LOG_ERROR("Failed to create rasterizer state");
		return 0;
	}

	return GetBitMask(cull_mode, fill_mode);
}

void RasterizerState::Clear()
{
	SAFE_RELEASE(state);
}

void RasterizerState::BindPipeline()
{
	graphics->GetDeviceContext()->RSSetState(state);
}
