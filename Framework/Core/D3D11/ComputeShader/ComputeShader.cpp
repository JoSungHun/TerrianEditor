#include "Framework.h"
#include "ComputeShader.h"
#include "../D3D11_Helper.h"

ComputeShader::ComputeShader(Context * context)
	: shader(nullptr)
	, path("")
	, entryPoint("")
	, shaderModel("")
{
	graphics = context->GetSubsystem<Graphics>();
}

ComputeShader::~ComputeShader()
{
	Clear();
}

void ComputeShader::Create(const std::string & path, const std::string & entryPoint, const std::string & shaderModel, D3D_SHADER_MACRO * defines)
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

	auto hr = graphics->GetDevice()->CreateComputeShader
	(
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		nullptr,
		&shader
	);
	assert(SUCCEEDED(hr));
	SAFE_RELEASE(blob);
}

void ComputeShader::Clear()
{
	SAFE_RELEASE(shader);
}

void ComputeShader::BindPipeline()
{
	graphics->GetDeviceContext()->CSSetShader(shader, nullptr, 0);
}
