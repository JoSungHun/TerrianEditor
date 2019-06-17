#pragma once
#include "../IStage.h"

class ComputeShader final : public IStage
{
public:
	ComputeShader(class Context* context);
	~ComputeShader();

	ComputeShader(const ComputeShader& rhs) = delete;
	ComputeShader& operator=(const ComputeShader& rhs) = delete;

	auto GetShader() const -> ID3D11ComputeShader* { return shader; }

	void Create
	(
		const std::string& path,
		const std::string& entryPoint = "CS",
		const std::string& shaderModel = "cs_5_0",
		D3D_SHADER_MACRO* defines = nullptr
	);
	void Clear();
	void BindPipeline();

private:
	Graphics* graphics;
	ID3D11ComputeShader* shader;
	std::string path;
	std::string entryPoint;
	std::string shaderModel;
};