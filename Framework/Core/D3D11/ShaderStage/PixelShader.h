#pragma once
#include "../IStage.h"

class PixelShader final : public IStage
{
public:
	PixelShader(class Context* context);
	~PixelShader();

	PixelShader(const PixelShader& rhs) = delete;
	PixelShader& operator=(const PixelShader& rhs) = delete;

	auto GetShader() const -> ID3D11PixelShader* { return shader; }

	void Create
	(
		const std::string& path,
		const std::string& entryPoint = "PS",
		const std::string& shaderModel = "ps_5_0",
		D3D_SHADER_MACRO* defines = nullptr
	);
	void Clear();
	void BindPipeline();

private:
	Graphics* graphics;
	ID3D11PixelShader* shader;
	std::string path;
	std::string entryPoint;
	std::string shaderModel;
};