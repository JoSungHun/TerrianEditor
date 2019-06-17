#pragma once
#include "../IStage.h"

class BlendState final : public IStage
{
public:
	static auto GetBitMask
	(
		const bool& blend_enable,
		const Factor& src_blend,
		const Factor& dest_blend,
		const Operation& blend_op,
		const Factor& src_blend_alpha,
		const Factor& dest_blend_alpha,
		const Operation& blend_op_alpha,
		const ColorMask& color_mask
	)-> const uint;

public:
	BlendState(class Context* context);
	~BlendState();

	BlendState(const BlendState&) = delete;
	BlendState& operator= (const BlendState&) = delete;

	auto GetState() const-> ID3D11BlendState* { return state; }

	auto Create
	(
		const bool& blend_enable,
		const Factor& src_blend,
		const Factor& dest_blend,
		const Operation& blend_op,
		const Factor& src_blend_alpha,
		const Factor& dest_blend_alpha,
		const Operation& blend_op_alpha,
		const ColorMask& color_mask = ColorMask::ALL
	)-> const uint;
	void Clear();
	void BindPipeline();

private:
	class Graphics* graphics;
	ID3D11BlendState* state;

	static std::map<Factor, D3D11_BLEND> blend_factors;
	static std::map<Operation, D3D11_BLEND_OP> blend_operations;
	static std::map<ColorMask, D3D11_COLOR_WRITE_ENABLE> blend_color_masks;

};