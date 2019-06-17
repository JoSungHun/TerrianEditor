#pragma once
#include "../IStage.h"

class VertexShader final : public IStage
{
public:
	VertexShader(class Context* context);
	~VertexShader();

	VertexShader(const VertexShader& rhs) = delete;
	VertexShader& operator=(const VertexShader& rhs) = delete;

	auto GetBlob() const -> ID3DBlob* { return blob; }
	auto GetShader() const -> ID3D11VertexShader* { return shader; }

	void Create
	(
		const std::string& path,
		const std::string& entryPoint = "VS",
		const std::string& shaderModel = "vs_5_0",
		D3D_SHADER_MACRO* defines = nullptr
	);
	void Clear();
	void BindPipeline();

private:
	Graphics* graphics;
	ID3D11VertexShader* shader;
	ID3DBlob* blob;
	std::string path;
	std::string entryPoint;
	std::string shaderModel;
};