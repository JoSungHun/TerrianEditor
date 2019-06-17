#include "Framework.h"
#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(Context * context)
	: buffer(nullptr)
	, stride(0)
	, offset(0)
	, vertexCount(0)
{
	graphics = context->GetSubsystem<Graphics>();
}

VertexBuffer::~VertexBuffer()
{
	Clear();
}

void VertexBuffer::UpdateBuffer(const void * data, uint size)
{
	graphics->GetDeviceContext()->UpdateSubresource
	(
		buffer,
		0,
		nullptr,
		data,
		size,
		0
	);
}

auto VertexBuffer::Map() -> void *
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto hr = graphics->GetDeviceContext()->Map
	(
		buffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedResource
	);
	assert(SUCCEEDED(hr));

	return mappedResource.pData;
}

void VertexBuffer::Unmap()
{
	graphics->GetDeviceContext()->Unmap(buffer, 0);
}

void VertexBuffer::Clear()
{
	stride = 0;
	offset = 0;
	vertexCount = 0;

	SAFE_RELEASE(buffer);
}

void VertexBuffer::BindPipeline()
{
	graphics->GetDeviceContext()->IASetVertexBuffers
	(
		0,
		1,
		&buffer,
		&stride,
		&offset
	);
}
