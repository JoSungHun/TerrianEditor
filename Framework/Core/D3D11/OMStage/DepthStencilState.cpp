#include "Framework.h"
#include "DepthStencilState.h"

std::map<ComparisonFunc, D3D11_COMPARISON_FUNC> DepthStencilState::comparison_funcs
{
	std::make_pair(ComparisonFunc::NEVER, D3D11_COMPARISON_NEVER),
	std::make_pair(ComparisonFunc::LESS, D3D11_COMPARISON_LESS),
	std::make_pair(ComparisonFunc::EQUAL, D3D11_COMPARISON_EQUAL),
	std::make_pair(ComparisonFunc::LESS_EQUAL, D3D11_COMPARISON_LESS_EQUAL),
	std::make_pair(ComparisonFunc::GREATER, D3D11_COMPARISON_GREATER),
	std::make_pair(ComparisonFunc::NOT_EQUAL,D3D11_COMPARISON_NOT_EQUAL),
	std::make_pair(ComparisonFunc::GREATER_EQUAL,D3D11_COMPARISON_GREATER_EQUAL),
	std::make_pair(ComparisonFunc::ALWAYS,D3D11_COMPARISON_ALWAYS),
};

auto DepthStencilState::GetBitMask(const bool & bDepth_enable, const ComparisonFunc & comparison) -> const uint
{
	return
		static_cast<uint>(bDepth_enable) |
		static_cast<uint>(comparison);
}

DepthStencilState::DepthStencilState(Context * context)
	: state(nullptr)
{
	graphics = context->GetSubsystem<Graphics>();
}

DepthStencilState::~DepthStencilState()
{
	Clear();
}

auto DepthStencilState::Create(const bool & bDepth_enable, const ComparisonFunc & comparison) -> const uint
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	if (bDepth_enable)
	{
		desc.DepthEnable					= true;
		desc.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc						= comparison_funcs[comparison];
		desc.StencilEnable					= false;
		desc.StencilReadMask				= D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask				= D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_REPLACE;
		desc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
		desc.BackFace						= desc.FrontFace;
	}
	else
	{
		desc.DepthEnable					= false;
		desc.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc						= comparison_funcs[comparison];
		desc.StencilEnable					= false;
		desc.StencilReadMask				= D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask				= D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_REPLACE;
		desc.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
		desc.BackFace						= desc.FrontFace;
	}

	auto result = SUCCEEDED(graphics->GetDevice()->CreateDepthStencilState(&desc, &state));
	if (!result)
	{
		LOG_ERROR("Failed to create depthStencilState");
		return 0;
	}

	return GetBitMask(bDepth_enable, comparison);
}

void DepthStencilState::Clear()
{
	SAFE_RELEASE(state);
}

void DepthStencilState::BindPipeline()
{
	graphics->GetDeviceContext()->OMSetDepthStencilState(state, 1);
}
