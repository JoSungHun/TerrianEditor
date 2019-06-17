#pragma once

class InstanceBuffer final
{
public:
	InstanceBuffer(class Context* context);
	~InstanceBuffer();

	ID3D11Buffer* GetBuffer() const { return buffer; }
	const uint& GetInstanceCount() const { return instanceCount; }
	const uint& GetStride() const { return stride; }
	const uint& GetOffset() const { return offset; }

	template<typename T>
	void Create
	(
		const std::vector<T>& instanceDatas,
		const D3D11_USAGE& usage = D3D11_USAGE_DYNAMIC
	);

	template<typename T>
	void Update
	(
		const T* instanceDatas,
		const uint size
	);

	void Clear();


private:
	class Graphics* graphics;

	ID3D11Buffer* buffer;
	uint stride;
	uint offset;
	uint instanceCount;
};

template<typename T>
inline void InstanceBuffer::Create(const std::vector<T>& instanceDatas, const D3D11_USAGE & usage)
{
	stride = sizeof(T);
	instanceCount = instanceDatas.size();

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	desc.Usage = usage;
	switch (usage)
	{
	case D3D11_USAGE_DEFAULT:
		desc.CPUAccessFlags = 0;
		break;
	case D3D11_USAGE_DYNAMIC:
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	case D3D11_USAGE_STAGING:
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		break;
	}

	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.ByteWidth = stride * instanceCount;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&subData, sizeof(D3D11_SUBRESOURCE_DATA));
	subData.pSysMem = instanceDatas.data();


	auto hr = graphics->GetDevice()->CreateBuffer(&desc, &subData, &buffer);
	assert(SUCCEEDED(hr));
}

template<typename T>
inline void InstanceBuffer::Update(const T * instanceDatas, const uint size)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	graphics->GetDeviceContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	memcpy_s(mappedResource.pData, sizeof(T)*size, instanceDatas, sizeof(T)*size);

	graphics->GetDeviceContext()->Unmap(buffer, 0);
}
