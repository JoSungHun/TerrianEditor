#pragma once

class IShader
{
public:
	IShader(class Context* context);
	virtual ~IShader();

	virtual void BindPipeline() = 0;

protected:
	const bool ShaderErrorHandler(const HRESULT& hr, ID3DBlob* error);

protected:
	class Graphics* graphics;
	
	std::string filePath;
	std::string functionName;
};