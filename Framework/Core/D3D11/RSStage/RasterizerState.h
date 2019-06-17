#pragma once
#include "../IStage.h"

class RasterizerState final : public IStage
{
public:
	static auto GetBitMask
	(
		const CullMode& cull_mode,
		const FillMode& fill_mode
	) -> const uint;

public:
	RasterizerState(class Context* context);
	~RasterizerState();

	RasterizerState(const RasterizerState&) = delete;
	RasterizerState& operator=(const RasterizerState&) = delete;

	auto GetState() const -> ID3D11RasterizerState* { return state; }

	auto Create
	(
		const CullMode& cull_mode,
		const FillMode& fill_mode
	) -> const uint;
	void Clear();
	void BindPipeline();

private:
	class Graphics* graphics;
	ID3D11RasterizerState* state;

private:
	static std::map<CullMode, D3D11_CULL_MODE> cull_modes;
	static std::map<FillMode, D3D11_FILL_MODE> fill_modes;
};