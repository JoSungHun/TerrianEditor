 #pragma once


enum class COMPUTE_BUFFER_TYPE
{
	INPUT,
	OUTPUT
};

class ComputeBuffer final
{
public:
	ComputeBuffer(class Context* context);
	ComputeBuffer(class Context* context, const COMPUTE_BUFFER_TYPE& type);
	~ComputeBuffer();



	void Clear();
	void BindPipeline(const uint& slot);
	void* GetResult();

	void SetBufferType(const COMPUTE_BUFFER_TYPE& type) { this->type = type; }
	const uint& GetSize() const { return dataCount; }


	template<typename T>
	void CreateBuffer(const T* datas = nullptr, const uint& size = 0);

	template<typename T>
	void UpdateBuffer(const T* datas = nullptr, const uint& size = 0);

private:
	class Graphics* graphics;
	COMPUTE_BUFFER_TYPE type;


	//Input
	ID3D11Buffer* buffer;
	union {
		ID3D11ShaderResourceView* bufferSrv;
		ID3D11UnorderedAccessView* bufferUAV;
	};
	uint stride;
	uint offset;
	uint dataCount;
	
};

template<typename T>
inline void ComputeBuffer::CreateBuffer(const T* datas, const uint& size)
{
	if (buffer)
	{
		SAFE_RELEASE(buffer);
		SAFE_RELEASE(bufferSrv);
		SAFE_RELEASE(bufferUAV);
	}

	stride = sizeof(T);
	dataCount = size;

	//Create Buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));

	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth = stride * dataCount;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = stride;
	if (type == COMPUTE_BUFFER_TYPE::INPUT)
	{
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (type == COMPUTE_BUFFER_TYPE::OUTPUT)
	{
		bufferDesc.CPUAccessFlags = 0;
	}


	if (datas)
	{
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = datas;
	
		auto hr = graphics->GetDevice()->CreateBuffer(&bufferDesc, &initData, &buffer);
		assert(SUCCEEDED(hr));
	}
	else {
		auto hr = graphics->GetDevice()->CreateBuffer(&bufferDesc, nullptr, &buffer);
		assert(SUCCEEDED(hr));
	}

//INPUT
	if (type == COMPUTE_BUFFER_TYPE::INPUT)
	{
		//Create SRV
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.FirstElement = 0;

		if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// structured buffer
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.BufferEx.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
		}
		else if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		{
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			srvDesc.BufferEx.NumElements = bufferDesc.ByteWidth / 4;
		}
		else assert(E_INVALIDARG);

		graphics->GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &bufferSrv);
	}
	else // else = output
	{
		//Output
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;

		if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			//structured buffer
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;
		}
		else if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		{
			uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / 4;
		}
		else assert(E_INVALIDARG);

		graphics->GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &bufferUAV);
	}

}


template<typename T>
inline void ComputeBuffer::UpdateBuffer(const T* datas, const uint& size)
{
	if (buffer == nullptr || dataCount != size || type == COMPUTE_BUFFER_TYPE::OUTPUT)
	{
		CreateBuffer(datas, size);
		return;
	}


	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if (type == COMPUTE_BUFFER_TYPE::INPUT)
	{
		auto hr = graphics->GetDeviceContext()->Map
		(
			reinterpret_cast<ID3D11Resource*>(buffer),
			0,
			D3D11_MAP_WRITE,
			0,
			&mappedResource
		);
		assert(SUCCEEDED(hr));


		memcpy_s(mappedResource.pData, sizeof(T)*size, datas, sizeof(T)*size);

		graphics->GetDeviceContext()->Unmap(buffer, 0);
	}
}