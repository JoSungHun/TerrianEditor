#pragma once
#include "../IStage.h"

class HullShader final : public IStage
{
public:
	HullShader(class Context* context);
	~HullShader();

	HullShader(const HullShader& rhs) = delete;
	HullShader& operator=(const HullShader& rhs) = delete;

	auto GetBlob() const -> ID3DBlob* { return blob; }
	auto GetShader() const -> ID3D11HullShader* { return shader; }

	void Create
	(
		const std::string& path,
		const std::string& entryPoint = "HS",
		const std::string& shaderModel = "hs_5_0",
		D3D_SHADER_MACRO* defines = nullptr
	);
	void Clear();
	void BindPipeline();

private:
	Graphics* graphics;
	ID3D11HullShader* shader;
	ID3DBlob* blob;
	std::string path;
	std::string entryPoint;
	std::string shaderModel;
};