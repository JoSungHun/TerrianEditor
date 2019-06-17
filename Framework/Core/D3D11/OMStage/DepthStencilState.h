#pragma once
#include "../IStage.h"

class DepthStencilState final : public IStage
{
public:
	static auto GetBitMask
	(
		const bool& bDepth_enable,
		const ComparisonFunc& comparison
	)-> const uint;

public:
	DepthStencilState(class Context* context);
	~DepthStencilState();

	DepthStencilState(const DepthStencilState&) = delete;
	DepthStencilState& operator=(const DepthStencilState&) = delete;

	auto GetState() const -> ID3D11DepthStencilState* { return state; }

	auto Create
	(
		const bool& bDepth_enable,
		const ComparisonFunc& comparison = Settings::Get().IsReverseZ() ? ComparisonFunc::GREATER_EQUAL : ComparisonFunc::LESS_EQUAL
	) -> const uint;

	void Clear();
	void BindPipeline();

private:
	class Graphics* graphics;
	ID3D11DepthStencilState* state;

	static std::map<ComparisonFunc, D3D11_COMPARISON_FUNC> comparison_funcs;
};