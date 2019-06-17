#include "Framework.h"
#include "BlendState.h"

std::map<Factor, D3D11_BLEND> BlendState::blend_factors
{
	std::make_pair(Factor::ZERO				, D3D11_BLEND_ZERO),
	std::make_pair(Factor::ONE				, D3D11_BLEND_ONE),
	std::make_pair(Factor::SRC_COLOR		, D3D11_BLEND_SRC_COLOR),
	std::make_pair(Factor::INV_SRC_COLOR	, D3D11_BLEND_INV_SRC_COLOR),
	std::make_pair(Factor::SRC_ALPHA		, D3D11_BLEND_SRC_ALPHA),
	std::make_pair(Factor::INV_SRC_ALPHA	, D3D11_BLEND_INV_SRC_ALPHA),
	std::make_pair(Factor::DEST_ALPHA		, D3D11_BLEND_DEST_ALPHA),
	std::make_pair(Factor::INV_DEST_ALPHA	, D3D11_BLEND_INV_DEST_ALPHA),
	std::make_pair(Factor::DEST_COLOR		, D3D11_BLEND_DEST_COLOR),
	std::make_pair(Factor::INV_DEST_COLOR	, D3D11_BLEND_INV_DEST_COLOR),
};

std::map<Operation, D3D11_BLEND_OP> BlendState::blend_operations
{
	std::make_pair(Operation::ADD			, D3D11_BLEND_OP_ADD),
	std::make_pair(Operation::SUBTRACT		, D3D11_BLEND_OP_SUBTRACT),
	std::make_pair(Operation::REV_SUBTRACT	, D3D11_BLEND_OP_REV_SUBTRACT),
	std::make_pair(Operation::MIN			, D3D11_BLEND_OP_MIN),
	std::make_pair(Operation::MAX			, D3D11_BLEND_OP_MAX),
};

std::map<ColorMask, D3D11_COLOR_WRITE_ENABLE> BlendState::blend_color_masks
{
	std::make_pair(ColorMask::RED		, D3D11_COLOR_WRITE_ENABLE_RED),
	std::make_pair(ColorMask::GREEN		, D3D11_COLOR_WRITE_ENABLE_GREEN),
	std::make_pair(ColorMask::BLUE		, D3D11_COLOR_WRITE_ENABLE_BLUE),
	std::make_pair(ColorMask::ALPHA		, D3D11_COLOR_WRITE_ENABLE_ALPHA),
	std::make_pair(ColorMask::ALL		, D3D11_COLOR_WRITE_ENABLE_ALL),
};



auto BlendState::GetBitMask(const bool & blend_enable, const Factor & src_blend, const Factor & dest_blend, const Operation & blend_op, const Factor & src_blend_alpha, const Factor & dest_blend_alpha, const Operation & blend_op_alpha, const ColorMask & color_mask) -> const uint
{
	return	static_cast<uint>(blend_enable) |
			static_cast<uint>(src_blend) |
			static_cast<uint>(dest_blend) |
			static_cast<uint>(blend_op) |
			static_cast<uint>(src_blend_alpha) |
			static_cast<uint>(dest_blend_alpha) |
			static_cast<uint>(blend_op_alpha) |
			static_cast<uint>(color_mask);
}

BlendState::BlendState(Context * context)
	: state(nullptr)
{
	graphics = context->GetSubsystem<Graphics>();
}

BlendState::~BlendState()
{
	Clear();
}

auto BlendState::Create(const bool & blend_enable, const Factor & src_blend, const Factor & dest_blend, const Operation & blend_op, const Factor & src_blend_alpha, const Factor & dest_blend_alpha, const Operation & blend_op_alpha, const ColorMask & color_mask) -> const uint
{
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;
	desc.RenderTarget[0].BlendEnable = blend_enable;
	desc.RenderTarget[0].SrcBlend = blend_factors[src_blend];
	desc.RenderTarget[0].DestBlend = blend_factors[dest_blend];
	desc.RenderTarget[0].BlendOp = blend_operations[blend_op];
	desc.RenderTarget[0].SrcBlendAlpha = blend_factors[src_blend_alpha];
	desc.RenderTarget[0].DestBlendAlpha = blend_factors[dest_blend_alpha];
	desc.RenderTarget[0].BlendOpAlpha = blend_operations[blend_op_alpha];
	desc.RenderTarget[0].RenderTargetWriteMask = blend_color_masks[color_mask];

	auto result = SUCCEEDED(graphics->GetDevice()->CreateBlendState(&desc, &state));
	if (!result)
	{
		LOG_ERROR("Failed to create blend state");
		return 0;
	}

	return GetBitMask
	(
		blend_enable,
		src_blend,
		dest_blend,
		blend_op,
		src_blend_alpha,
		dest_blend_alpha,
		blend_op_alpha,
		color_mask
	);
}

void BlendState::Clear()
{
	SAFE_RELEASE(state);
}

void BlendState::BindPipeline()
{
	float blend_factor[4]{ 0.0f,0.0f,0.0f,0.0f };
	graphics->GetDeviceContext()->OMSetBlendState(state, blend_factor, 0xffffffff);
}
