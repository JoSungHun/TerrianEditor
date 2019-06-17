#pragma once
#include "Framework.h"
#include "ComputeBuffer.h"

ComputeBuffer::ComputeBuffer(Context * context)
	: buffer(nullptr)
	, bufferUAV(nullptr)
	, bufferSrv(nullptr)
	, stride(0)
	, offset(0)
	, dataCount(0)
{
	graphics = context->GetSubsystem<Graphics>();
}

ComputeBuffer::ComputeBuffer(Context * context, const COMPUTE_BUFFER_TYPE & type)
	: buffer(nullptr)
	, bufferUAV(nullptr)
	, bufferSrv(nullptr)
	, stride(0)
	, offset(0)
	, dataCount(0)
{
	graphics = context->GetSubsystem<Graphics>();
	this->type = type;
}

ComputeBuffer::~ComputeBuffer()
{
}

void ComputeBuffer::Clear()
{
	SAFE_RELEASE(buffer);
}

void ComputeBuffer::BindPipeline(const uint& slot)
{
	if (type == COMPUTE_BUFFER_TYPE::INPUT)
	{
		graphics->GetDeviceContext()->CSSetShaderResources(slot, 1, &bufferSrv);
	}
	else
	{
		graphics->GetDeviceContext()->CSSetUnorderedAccessViews(slot, 1, &bufferUAV, nullptr);
	}
}

void * ComputeBuffer::GetResult()
{
	ID3D11Buffer* copyBuffer = nullptr;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	buffer->GetDesc(&desc);

	uint byteSize = desc.ByteWidth;

	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	auto hr = graphics->GetDevice()->CreateBuffer(&desc, nullptr, &copyBuffer);
	assert(SUCCEEDED(hr));

	graphics->GetDeviceContext()->CopyResource(copyBuffer, buffer);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = graphics->GetDeviceContext()->Map
	(
		copyBuffer,
		0,
		D3D11_MAP_READ,
		0,
		&mappedResource
	);
	assert(SUCCEEDED(hr));

	byte* resultData = new byte[byteSize];
	memcpy_s(resultData, byteSize, mappedResource.pData, byteSize);

	graphics->GetDeviceContext()->Unmap(copyBuffer, 0);

	copyBuffer->Release();

	return resultData;
}
