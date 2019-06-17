#pragma once
#include "../IStage.h"

class Sampler final : public IStage
{
public:
	Sampler(class Context* context);
	~Sampler();

	Sampler(const Sampler&) = delete;
	Sampler& operator=(const Sampler&) = delete;

	auto GetState() const -> ID3D11SamplerState* { return state; }

	auto Create
	(
		const D3D11_FILTER& filter = D3D11_FILTER_ANISOTROPIC,
		const D3D11_TEXTURE_ADDRESS_MODE& address_mode = D3D11_TEXTURE_ADDRESS_WRAP,
		const D3D11_COMPARISON_FUNC& comparison = D3D11_COMPARISON_ALWAYS
	)-> const uint;
	void Clear();
	void BindPipeline(const ShaderStage& stage, const uint& slot);

private:
	class Graphics* graphics;
	ID3D11SamplerState* state;
};