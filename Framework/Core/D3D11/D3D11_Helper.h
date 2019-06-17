#pragma once
#include "Framework.h"

namespace D3D11_Helper
{
	inline const bool ShaderErrorHandler(const HRESULT & hr, ID3DBlob * error)
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

	inline const bool CompileShader
	(
		const std::string& path,
		const std::string& entryPoint,
		const std::string& shaderModel,
		D3D_SHADER_MACRO* defines,
		ID3DBlob** blob
	)
	{
#ifndef OPTIMIZATION
		uint flags = D3DCOMPILE_ENABLE_STRICTNESS;
#else
		uint flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3|D3D10_SHADER_DEBUG;
#endif

		ID3DBlob* error = nullptr;
		auto hr = D3DCompileFromFile
		(
			FileSystem::ToWstring(path).c_str(),
			defines,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entryPoint.c_str(),
			shaderModel.c_str(),
			flags,
			0,
			blob,
			&error
		);
		auto result = ShaderErrorHandler(hr, error);

		SAFE_RELEASE(error);

		return result;
	}
}