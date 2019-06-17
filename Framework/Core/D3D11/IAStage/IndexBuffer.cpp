#include "Framework.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Context * context)
	: buffer(nullptr)
	, offset(0)
	, indexCount(0)
{
	graphics = context->GetSubsystem<Graphics>();
}

IndexBuffer::~IndexBuffer()
{
	Clear();
}

void IndexBuffer::Create(const std::vector<uint>& indices, const D3D11_USAGE & usage)
{
	assert(!indices.empty());

	Clear();
	indexCount = indices.size();

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

	switch (usage)
	{
	case D3D11_USAGE_DEFAULT:
		desc.CPUAccessFlags = 0;
		break;
	case D3D11_USAGE_DYNAMIC:
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	}

	desc.Usage = usage;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof(uint) * indices.size();

	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&subData, sizeof(D3D11_SUBRESOURCE_DATA));
	subData.pSysMem = indices.data();

	auto hr = graphics->GetDevice()->CreateBuffer
	(
		&desc,
		&subData,
		&buffer
	);
	assert(SUCCEEDED(hr));
}

void IndexBuffer::Clear()
{
	offset = 0;
	indexCount = 0;

	SAFE_RELEASE(buffer);
}

void IndexBuffer::BindPipeline()
{
	graphics->GetDeviceContext()->IASetIndexBuffer
	(
		buffer,
		DXGI_FORMAT_R32_UINT,
		offset
	);
}
