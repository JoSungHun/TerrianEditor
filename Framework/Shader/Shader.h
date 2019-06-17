#pragma once

class Shader final
{
public:
	Shader(Context* context);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	auto GetShaderID() const -> const uint& { return shaderID; }

	auto GetVertexShader()->VertexShader* const;
	auto GetPixelShader()->PixelShader* const;
	auto GetInputLayout()-> InputLayout* const { return layout; }

	void AddShader
	(
		const ShaderStage& type,
		const std::string& path,
		const std::string& entryPoint = "",
		const std::string& shaderModel = ""
	);

	void AddDefine
	(
		const std::string& define,
		const std::string& value = "1"
	);

	void Clear();
	void Clear(const ShaderStage& type);

private:
	Context* context;
	InputLayout* layout;

	uint shaderID;
	std::map<ShaderStage, void*> shaders;
	std::map<std::string, std::string> defines;
};