#include "Framework.h"
#include "IShader.h"

IShader::IShader(Context * context)
	: filePath("")
	, functionName("")
{
	graphics = context->GetSubsystem<Graphics>();
}

IShader::~IShader()
{
}

const bool IShader::ShaderErrorHandler(const HRESULT & hr, ID3DBlob * error)
{
	if (FAILED(hr))
	{
		if (error)
		{
			std::string str = reinterpret_cast<const char*>(error->GetBufferPointer());
			MessageBoxA
			(
				nullptr,
				str.c_str(),
				"Shader Error!!!!!!!",
				MB_OK
			);
		}
		return false;
	}
	return true;
}
