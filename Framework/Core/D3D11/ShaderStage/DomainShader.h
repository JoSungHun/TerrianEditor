#pragma once
#include "../IStage.h"

class DomainShader final : public IStage
{
public:
	DomainShader(class Context* context);
	~DomainShader();

	DomainShader(const DomainShader& rhs) = delete;
	DomainShader& operator=(const DomainShader& rhs) = delete;

	auto GetBlob() const -> ID3DBlob* { return blob; }
	auto GetShader() const -> ID3D11DomainShader* { return shader; }

	void Create
	(
		const std::string& path,
		const std::string& entryPoint = "DS",
		const std::string& shaderModel = "ds_5_0",
		D3D_SHADER_MACRO* defines = nullptr
	);
	void Clear();
	void BindPipeline();

private:
	Graphics* graphics;
	ID3D11DomainShader* shader;
	ID3DBlob* blob;
	std::string path;
	std::string entryPoint;
	std::string shaderModel;
};