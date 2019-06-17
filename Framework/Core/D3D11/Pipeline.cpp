#include "Framework.h"
#include "Pipeline.h"

Pipeline::Pipeline(Context * context)
	: context(context)
	, vertexBuffer(nullptr)
	, instanceBuffer(nullptr)
	, indexBuffer(nullptr)
	, inputLayout(nullptr)
	, vertexShader(nullptr)
	, hullShader(nullptr)
	, domainShader(nullptr)
	, pixelShader(nullptr)
	, rsState(nullptr)
{
	graphics = context->GetSubsystem<Graphics>();

	rsState = new RasterizerState(context);
}

Pipeline::~Pipeline()
{
	SAFE_DELETE(vertexShader);
	SAFE_DELETE(hullShader);
	SAFE_DELETE(domainShader);
	SAFE_DELETE(pixelShader);
	//SAFE_DELETE(geometryShader);

	for (auto cBuffer : vsConstantBuffers)
	{
		SAFE_DELETE(cBuffer);
	}
	vsConstantBuffers.shrink_to_fit();
	for (auto cBuffer : hsConstantBuffers)
	{
		SAFE_DELETE(cBuffer);
	}
	hsConstantBuffers.shrink_to_fit();
	for (auto cBuffer : dsConstantBuffers)
	{
		SAFE_DELETE(cBuffer);
	}
	dsConstantBuffers.shrink_to_fit();
	for (auto cBuffer : psConstantBuffers)
	{
		SAFE_DELETE(cBuffer);
	}
	psConstantBuffers.shrink_to_fit();

}

void Pipeline::SetInputLayout(D3D11_INPUT_ELEMENT_DESC* descs, const uint& count)
{
	if (inputLayout)
		SAFE_DELETE(inputLayout);

	inputLayout = new InputLayout(context);
	inputLayout->Create(descs, count, vertexShader->GetBlob());
}

void Pipeline::SetShader(const ShaderType & type, const std::string & shaderFileName)
{
	switch (type)
	{
	case ShaderType::VS:
		if (vertexShader)
			SAFE_DELETE(vertexShader);
		vertexShader = new VertexShader(context);
		vertexShader->Create(shaderFileName);
		inputLayout = new InputLayout(context);
		inputLayout->Create(vertexShader->GetBlob());
		break;
	case ShaderType::HS:
		if (hullShader)
			SAFE_DELETE(hullShader);
		hullShader = new HullShader(context);
		hullShader->Create(shaderFileName);
		break;
	case ShaderType::DS:
		if (domainShader)
			SAFE_DELETE(domainShader);
		domainShader = new DomainShader(context);
		domainShader->Create(shaderFileName);
		break;
	case ShaderType::GS:
		//shader = new GeometryShader(context);
		break;
	case ShaderType::PS:
		if (pixelShader)
			SAFE_DELETE(pixelShader);
		pixelShader = new PixelShader(context);
		pixelShader->Create(shaderFileName);
		break;
	default:
		return;
		break;
	}
}

void Pipeline::SetSRVSlot(const ShaderType & type, const uint & numSRVSlot)
{
	if (numSRVSlot == 0)
		return;

	switch (type)
	{
	case ShaderType::VS:
		vsShaderResourceViews.resize(numSRVSlot);
		break;
	case ShaderType::HS:
		hsShaderResourceViews.resize(numSRVSlot);
		break;
	case ShaderType::DS:
		dsShaderResourceViews.resize(numSRVSlot);
		break;
	case ShaderType::GS:
		//gsShaderResourceViews.resize(numSRVSlot);
		break;
	case ShaderType::PS:
		psShaderResourceViews.resize(numSRVSlot);
		break;
	default:
		break;
	}
}

void Pipeline::SetSRVSlot(const uint & vsSlot, const uint & psSlot, const uint & hsSlot, const uint & dsSlot, const uint & gsSlot)
{
	SetSRVSlot(ShaderType::VS, vsSlot);
	SetSRVSlot(ShaderType::PS, psSlot);
	SetSRVSlot(ShaderType::HS, hsSlot);
	SetSRVSlot(ShaderType::DS, dsSlot);
	SetSRVSlot(ShaderType::GS, gsSlot); 
}

void Pipeline::SetShaderResourceView(const ShaderType & type, const uint & slotNum, ID3D11ShaderResourceView * srv)
{
	switch (type)
	{
	case ShaderType::VS:
		vsShaderResourceViews[slotNum] = srv;
		break;
	case ShaderType::HS:
		hsShaderResourceViews[slotNum] = srv;
		break;
	case ShaderType::DS:
		dsShaderResourceViews[slotNum] = srv;
		break;
	case ShaderType::GS:
		//gsShaderResoruceViews[slotNum] = srv;
		break;
	case ShaderType::PS:
		psShaderResourceViews[slotNum] = srv;
		break;
	default:
		break;
	}
}

void Pipeline::SetShaderResourceView(const ShaderType & type, const uint & slotNum, Texture * texture)
{
	switch (type)
	{
	case ShaderType::VS:
		vsShaderResourceViews[slotNum] = texture->GetSRV();
		break;
	case ShaderType::HS:
		hsShaderResourceViews[slotNum] = texture->GetSRV();
		break;
	case ShaderType::DS:
		dsShaderResourceViews[slotNum] = texture->GetSRV();
		break;
	case ShaderType::GS:
		//gsShaderResoruceViews[slotNum] = texture->GetSRV();
		break;
	case ShaderType::PS:
		psShaderResourceViews[slotNum] = texture->GetSRV();
		break;
	default:
		break;
	}
}

void Pipeline::SetShaderResourceView(const ShaderType & type, const uint & slotNum, const std::string & fileName)
{
	//TODO : Create ResourceManager and get srv from fileName
}

void Pipeline::SetRSCullMode(const D3D11_CULL_MODE & cullmode)
{
	rsState->CullMode(cullmode);
}

void Pipeline::SetRSFillMode(const D3D11_FILL_MODE & fillMode)
{
	rsState->FillMode(fillMode);
}

void Pipeline::Bind()
{
	if (instanceBuffer)
	{
		ID3D11Buffer* arrBuffer[2] = { vertexBuffer->GetBuffer(), instanceBuffer->GetBuffer() };
		uint arrStride[2] = { vertexBuffer->GetStride(),instanceBuffer->GetStride() };
		uint arrOffset[2] = { vertexBuffer->GetOffset(),instanceBuffer->GetOffset() };
		graphics->GetDeviceContext()->IASetVertexBuffers(0, 2, arrBuffer, arrStride, arrOffset);
	}
	else
	{
		vertexBuffer->BindPipeline();
	}
	indexBuffer->BindPipeline();
	inputLayout->BindPipeline();
	graphics->GetDeviceContext()->IASetPrimitiveTopology(topology);

	for (uint i = 0; i < vsConstantBuffers.size(); ++i)
		vsConstantBuffers[i]->buffer->BindPipeline(ShaderType::VS, i);
	for (uint i = 0; i < vsShaderResourceViews.size(); ++i)
		graphics->GetDeviceContext()->VSSetShaderResources(i, 1, &vsShaderResourceViews[i]);

	for (uint i = 0; i < hsConstantBuffers.size(); ++i)
		hsConstantBuffers[i]->buffer->BindPipeline(ShaderType::HS, i);
	for (uint i = 0; i < hsShaderResourceViews.size(); ++i)
		graphics->GetDeviceContext()->HSSetShaderResources(i, 1, &hsShaderResourceViews[i]);

	for (uint i = 0; i < dsConstantBuffers.size(); ++i)
		dsConstantBuffers[i]->buffer->BindPipeline(ShaderType::DS, i);
	for (uint i = 0; i < dsShaderResourceViews.size(); ++i)
		graphics->GetDeviceContext()->DSSetShaderResources(i, 1, &dsShaderResourceViews[i]);

	for (uint i = 0; i < psConstantBuffers.size(); ++i)
		psConstantBuffers[i]->buffer->BindPipeline(ShaderType::PS, i);
	for (uint i = 0; i < psShaderResourceViews.size(); ++i)
		graphics->GetDeviceContext()->PSSetShaderResources(i, 1, &psShaderResourceViews[i]);
	 
	if(vertexShader)
		vertexShader->BindPipeline();
	if(hullShader)
		hullShader->BindPipeline();
	if(domainShader)
		domainShader->BindPipeline();
	if(pixelShader)
		pixelShader->BindPipeline();
	if(rsState)
		rsState->BindPipeline();
}

void Pipeline::UnBind()
{ 
	if (rsState)
		rsState->UnbindPipeline();

	if(domainShader || hullShader)
		graphics->TurnBackOffShaders();
}
