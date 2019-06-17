#include "Framework.h"
#include "PixelShader.h"
#include "../D3D11_Helper.h"

PixelShader::PixelShader(Context * context)
	: shader(nullptr)
	, path("")
	, entryPoint("")
	, shaderModel("")
{
	graphics = context->GetSubsystem<Graphics>();
}

PixelShader::~PixelShader()
{
	Clear();
}

void PixelShader::Create(const std::string & path, const std::string & entryPoint, const std::string & shaderModel, D3D_SHADER_MACRO * defines)
{
	this->path = path;
	this->entryPoint = entryPoint;
	this->shaderModel = shaderModel;

	ID3DBlob* blob = nullptr;
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

	auto hr = graphics->GetDevice()->CreatePixelShader
	(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		&shader
	);
	assert(SUCCEEDED(hr));
	SAFE_RELEASE(blob);
}

void PixelShader::Clear()
{
	SAFE_RELEASE(shader);
}

void PixelShader::BindPipeline()
{
	graphics->GetDeviceContext()->PSSetShader(shader, nullptr, 0);
}
