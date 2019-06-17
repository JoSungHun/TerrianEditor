#include "Framework.h"
#include "HullShader.h"
#include "../D3D11_Helper.h"

HullShader::HullShader(Context * context)
	: shader(nullptr)
	, blob(nullptr)
	, path("")
	, entryPoint("")
	, shaderModel("")
{
	graphics = context->GetSubsystem<Graphics>();
}

HullShader::~HullShader()
{
	Clear();
}

void HullShader::Create(const std::string & path, const std::string & entryPoint, const std::string & shaderModel, D3D_SHADER_MACRO * defines)
{
	this->path = path;
	this->entryPoint = entryPoint;
	this->shaderModel = shaderModel;

	auto result = D3D11_Helper::CompileShader
	(
		path,
		entryPoint,
		shaderModel,
		defines,
		&blob
	);

	if (!result)
		assert(false);

	auto hr = graphics->GetDevice()->CreateHullShader
	(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		&shader
	);
	assert(SUCCEEDED(hr));
}

void HullShader::Clear()
{
	SAFE_RELEASE(shader);
	SAFE_RELEASE(blob);
}

void HullShader::BindPipeline()
{
	graphics->GetDeviceContext()->HSSetShader(shader, nullptr, 0);
}
