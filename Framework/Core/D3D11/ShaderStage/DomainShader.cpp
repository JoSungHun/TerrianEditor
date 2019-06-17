#include "Framework.h"
#include "DomainShader.h"
#include "../D3D11_Helper.h"

DomainShader::DomainShader(Context * context)
	: shader(nullptr)
	, blob(nullptr)
	, path("")
	, entryPoint("")
	, shaderModel("")
{
	graphics = context->GetSubsystem<Graphics>();
}

DomainShader::~DomainShader()
{
	Clear();
}

void DomainShader::Create(const std::string & path, const std::string & entryPoint, const std::string & shaderModel, D3D_SHADER_MACRO * defines)
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

	auto hr = graphics->GetDevice()->CreateDomainShader
	(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		&shader
	);
	assert(SUCCEEDED(hr));
}

void DomainShader::Clear()
{
	SAFE_RELEASE(shader);
	SAFE_RELEASE(blob);
}

void DomainShader::BindPipeline()
{
	graphics->GetDeviceContext()->DSSetShader(shader, nullptr, 0);
}
