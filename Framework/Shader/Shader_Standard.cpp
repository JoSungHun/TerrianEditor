#include "Framework.h"
#include "Shader_Standard.h"

std::map<ShaderFlags, Shader*> Shader_Standard::standardShaders;

auto Shader_Standard::GetMatchingStandardShader(Context * context, const ulong & flags) -> Shader *
{
	auto iter = standardShaders.insert
	(
		std::make_pair(static_cast<ShaderFlags>(flags), new Shader(context))
	);

	if (iter.second)
	{
		auto shader = iter.first->second;

		shader->AddDefine("ALBEDO_TEXTURE", flags & ShaderFlags_Albedo ? "1" : "0");
		shader->AddDefine("ROUGHNESS_TEXTURE", flags & ShaderFlags_Roughness ? "1" : "0");
		shader->AddDefine("METALLIC_TEXTURE", flags & ShaderFlags_Metallic ? "1" : "0");
		shader->AddDefine("NORMAL_TEXTURE", flags & ShaderFlags_Normal ? "1" : "0");
		shader->AddDefine("HEIGHT_TEXTURE", flags & ShaderFlags_Height ? "1" : "0");
		shader->AddDefine("OCCLUSION_TEXTURE", flags & ShaderFlags_Occlusion ? "1" : "0");
		shader->AddDefine("EMISSION_TEXTURE", flags & ShaderFlags_Emissive ? "1" : "0");
		shader->AddDefine("MASK_TEXTURE", flags & ShaderFlags_Mask ? "1" : "0");

		shader->AddShader(ShaderStage::PS, "../_Assets/Shader/GBuffer.hlsl");
	}

	return iter.first->second;
}
