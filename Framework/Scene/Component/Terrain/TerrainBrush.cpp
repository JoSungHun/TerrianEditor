#include "Framework.h"
#include "TerrainBrush.h"

TerrainBrush::TerrainBrush(Context * context)
	: context(context)
	, graphics(nullptr)
	, brushTex(nullptr)
	, splatTex(nullptr)
	, splatSrv(nullptr)
{
	graphics = context->GetSubsystem<Graphics>(); 
	inputBuffer = new ComputeBuffer(context, COMPUTE_BUFFER_TYPE::INPUT);
	outputBuffer = new ComputeBuffer(context, COMPUTE_BUFFER_TYPE::OUTPUT);
	outputTexture = new ComputeBuffer(context, COMPUTE_BUFFER_TYPE::OUTPUT);

	rowPitchBuffer = new ConstantBuffer(context);
	

	computeShader = new ComputeShader(context);
	computeShader->Create("../_Assets/Shader/TerrainBrush.hlsl");
	paintBuffer = new ConstantBuffer(context);
	paintBuffer->Create<PaintData>();
	paintData.Color = D3DXCOLOR(0, 1, 0, 0);
	
	for (uint i = 0; i < 4; i++)
		for (uint j = 0; j < 3; j++)
			splatChannel[i][j] = nullptr;
	
}

TerrainBrush::~TerrainBrush()
{
	SAFE_DELETE(inputBuffer);
	SAFE_DELETE(outputBuffer);
	SAFE_DELETE(computeShader);
	SAFE_DELETE(paintBuffer);

}

void TerrainBrush::Update(VertexPTNTBC* vertices, uint size)
{
	if (!active)
		return;

	if (brushTex == nullptr)
		return;

	inputBuffer->UpdateBuffer(vertices, size);
	outputBuffer->UpdateBuffer(vertices, size);

	auto data = static_cast<PaintData*>(paintBuffer->Map());
	(*data) = paintData;
	paintBuffer->Unmap();

	inputBuffer->BindPipeline(0);
	outputBuffer->BindPipeline(0);
	paintBuffer->BindPipeline(ShaderType::CS, 0);

	ID3D11ShaderResourceView* srv = brushTex->GetSRV();
	graphics->GetDeviceContext()->CSSetShaderResources(1, 1, &srv);


	if (paintData.BrushType == BrushType::BRUSH_TEXTURE)
	{
		SetInputTexture();
		//inputTexture->BindPipeline(2);
		uint trTextureSize = static_cast<uint>(paintData.trSize[0]) * static_cast<uint>(paintData.trSize[1]);
		outputTexture->CreateBuffer<D3DXCOLOR>(nullptr, trTextureSize);
		outputTexture->BindPipeline(1);
		graphics->GetDeviceContext()->CSSetShaderResources(2, 1, &splatSrv);
	}


	computeShader->BindPipeline();


	graphics->GetDeviceContext()->Dispatch(size, 1, 1);

	VertexPTNTBC* result = static_cast<VertexPTNTBC*>(outputBuffer->GetResult());
	/*std::vector<VertexPTNTBC> vTest(size);
	memcpy(vTest.data(), result, sizeof(VertexPTNTBC)*size);*/
	memcpy(vertices, result, sizeof(VertexPTNTBC)*size);
	SAFE_DELETE_ARRAY(result);
		 
	
	if (paintData.BrushType == BrushType::BRUSH_TEXTURE)
	{
		D3DXCOLOR* result = static_cast<D3DXCOLOR*>(outputTexture->GetResult());
		std::vector<D3DXCOLOR> resultColors(size);
		memcpy(resultColors.data(), result, sizeof(D3DXCOLOR)*size);
		SAFE_DELETE_ARRAY(result);

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		auto hr = graphics->GetDeviceContext()->Map(splatTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		assert(SUCCEEDED(hr));
		uint height = static_cast<uint>(paintData.trSize[1]);
		uint width = static_cast<uint>(paintData.trSize[0]);
		BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
		BYTE* buffer = reinterpret_cast<BYTE*>(resultColors.data());
		for (uint i = 0; i < height; ++i)
		{
			memcpy(mappedData, buffer, sizeof(D3DXCOLOR)*width);
			mappedData += mappedResource.RowPitch;
			buffer += sizeof(D3DXCOLOR)*width;
		}
		//memcpy(mappedResource.pData, resultColors.data(), sizeof(D3DXCOLOR)*colorSize);
		graphics->GetDeviceContext()->Unmap(splatTex, 0);

		

		if (splatSrv)
			SAFE_RELEASE(splatSrv);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		hr = graphics->GetDevice()->CreateShaderResourceView
		(
			splatTex,
			&srvDesc,
			&splatSrv
		);
		assert(SUCCEEDED(hr));
	}



	
	return;
}
//const BrushData& GetBrushData
const BrushData TerrainBrush::GetBrushData()
{
	BrushData brushData;
	brushData.Color = paintData.Color;
	brushData.Position = paintData.Position;
	brushData.Range = paintData.Range;
	return brushData;
}

ID3D11ShaderResourceView* TerrainBrush::GetBrushSRV()
{
	if (brushTex == nullptr)
		return nullptr;

	return brushTex->GetSRV();
	
}


void TerrainBrush::SetChannelTexture(uint channel, Texture* diffuse, Texture* normal, Texture* specular)
{
	splatChannel[channel][0] = diffuse;
	splatChannel[channel][1] = normal;
	splatChannel[channel][2] = specular; 
}

void TerrainBrush::SetInputTexture()
{
	if (splatTex == nullptr)
		CreateSplatTexture(static_cast<uint>(paintData.trSize[0]),static_cast<uint>(paintData.trSize[1]));

	/*ID3D11Texture2D* tempTexture;
	D3D11_TEXTURE2D_DESC desc;
	splatTex->GetDesc(&desc);

	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	auto hr = graphics->GetDevice()->CreateTexture2D(&desc, nullptr, &tempTexture);
	assert(SUCCEEDED(hr));

	graphics->GetDeviceContext()->CopyResource(tempTexture, splatTex);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = graphics->GetDeviceContext()->Map(tempTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
	assert(SUCCEEDED(hr));

	paintData.texRowPitch = mappedResource.RowPitch;

	uint textureDataSize = mappedResource.RowPitch*desc.Height/4;
	D3DXCOLOR* textureData = new D3DXCOLOR[textureDataSize];
	memcpy(textureData, mappedResource.pData, textureDataSize);
	graphics->GetDeviceContext()->Unmap(tempTexture, 0);

	inputTexture->UpdateBuffer(textureData, textureDataSize);*/

}

void TerrainBrush::CreateSplatTexture(const uint& width, const uint& height)
{
	///Create Texture2D
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texDesc.MiscFlags = 0;

	auto hr = graphics->GetDevice()->CreateTexture2D(&texDesc, nullptr, &splatTex);
	assert(SUCCEEDED(hr));

	/// Create Tex srv
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	
	hr = graphics->GetDevice()->CreateShaderResourceView(splatTex, &srvDesc, &splatSrv);

}

void TerrainBrush::LoadSplatTexture(std::string & fileName)
{
}

ID3D11ShaderResourceView * TerrainBrush::GetSplatChannel(uint channel, uint dns)
{
	if (splatChannel[channel][dns]) 
		return splatChannel[channel][dns]->GetSRV();
	
	
	return nullptr;
}


void TerrainBrush::SaveSplatMap()
{ 
	D3DX11SaveTextureToFileA
	(
		graphics->GetDeviceContext(),
		splatTex,
		D3DX11_IMAGE_FILE_FORMAT::D3DX11_IFF_BMP,
		"../_Assets/Saves/splatTex.bmp"
	);


}
