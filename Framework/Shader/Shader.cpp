#include "Framework.h"
#include "Shader.h"

Shader::Shader(Context * context)
	: context(context)
	, layout(nullptr)
{
	shaderID = GUIDGenerator::Generate();
}

Shader::~Shader()
{
	SAFE_DELETE(layout);

	for (auto& shader : shaders)
		SAFE_DELETE(shader.second);

	shaders.clear();
	defines.clear();
}

auto Shader::GetVertexShader() -> VertexShader * const
{
	if (!shaders.count(ShaderStage::VS))
		return nullptr;

	return reinterpret_cast<VertexShader*>(shaders[ShaderStage::VS]);
}

auto Shader::GetPixelShader() -> PixelShader * const
{
	if (!shaders.count(ShaderStage::PS))
		return nullptr;

	return reinterpret_cast<PixelShader*>(shaders[ShaderStage::PS]);
}

void Shader::AddShader(const ShaderStage & type, const std::string & path, const std::string & entryPoint, const std::string & shaderModel)
{
	std::vector<D3D_SHADER_MACRO> macros;
	for (auto& define : defines)
		macros.emplace_back(D3D_SHADER_MACRO{ define.first.c_str(), define.second.c_str() });

	macros.emplace_back(D3D_SHADER_MACRO{ nullptr, nullptr });

	switch (type)
	{
	case ShaderStage::VS:
	{
		//_Pairib == pair<iterator, bool>
		auto iter = shaders.insert(std::make_pair(type, new VertexShader(context)));
		if (iter.second)
		{
			auto shader = reinterpret_cast<VertexShader*>(iter.first->second);
			shader->Create
			(
				path,
				entryPoint.length() ? entryPoint : "VS",
				shaderModel.length() ? shaderModel : "vs_5_0",
				macros.data()
			);

			if (!layout)
			{
				layout = new InputLayout(context);
				layout->Create(shader->GetBlob());
			}
			else
			{
				layout->Clear();
				layout->Create(shader->GetBlob());
			}

		}
	}
	break;

	case ShaderStage::PS:
	{
		auto iter = shaders.insert(std::make_pair(type, new PixelShader(context)));
		if (iter.second)
		{
			auto shader = reinterpret_cast<PixelShader*>(iter.first->second);
			shader->Create
			(
				path,
				entryPoint.length() ? entryPoint : "PS",
				shaderModel.length() ? shaderModel : "ps_5_0",
				macros.data()
			);
		}
	}
	break;

	default:
		break;
	}
}

void Shader::AddDefine(const std::string & define, const std::string & value)
{
	defines[define] = value;
}

void Shader::Clear()
{
	for (const auto& shader : shaders)
		Clear(shader.first);
}

void Shader::Clear(const ShaderStage & type)
{
	switch (type)
	{
	case ShaderStage::VS:
		reinterpret_cast<VertexShader*>(shaders[type])->Clear();
		layout->Clear();
		break;
	case ShaderStage::HS:
		break;
	case ShaderStage::DS:
		break;
	case ShaderStage::GS:
		break;
	case ShaderStage::PS:
		reinterpret_cast<PixelShader*>(shaders[type])->Clear();
		break;
	case ShaderStage::CS:
		break;
	default:
		break;
	}
}