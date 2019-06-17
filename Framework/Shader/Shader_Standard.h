#pragma once

enum ShaderFlags : ulong
{
	ShaderFlags_Albedo = 1UL << 0,
	ShaderFlags_Roughness = 1UL << 1,
	ShaderFlags_Metallic = 1UL << 2,
	ShaderFlags_Normal = 1UL << 3,
	ShaderFlags_Height = 1UL << 4,
	ShaderFlags_Occlusion = 1UL << 5,
	ShaderFlags_Emissive = 1UL << 6,
	ShaderFlags_Mask = 1UL << 7,
};

class Shader_Standard final
{
public:
	static auto GetMatchingStandardShader
	(
		Context* context,
		const ulong& flags
	)->Shader*;

private:
	Shader_Standard() = default;
	~Shader_Standard() = default;

	Shader_Standard(const Shader_Standard&) = delete;
	Shader_Standard& operator=(const Shader_Standard&) = delete;

private:
	static std::map<ShaderFlags, Shader*> standardShaders;
};