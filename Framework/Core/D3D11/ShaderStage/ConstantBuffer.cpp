#include "Framework.h"
#include "ConstantBuffer.h"

ConstantBuffer::ConstantBuffer(Context * context)
	: buffer(nullptr)
{
	graphics = context->GetSubsystem<Graphics>();
}

ConstantBuffer::~ConstantBuffer()
{
	Clear();
}

void ConstantBuffer::Unmap()
{
	graphics->GetDeviceContext()->Unmap(buffer, 0);
}

void ConstantBuffer::Clear()
{
	SAFE_RELEASE(buffer);
}

void ConstantBuffer::BindPipeline(const ShaderStage & type, const uint & slot)
{
	switch (type)
	{
	case ShaderStage::VS: graphics->GetDeviceContext()->VSSetConstantBuffers(slot, 1, &buffer); break;
	case ShaderStage::PS: graphics->GetDeviceContext()->PSSetConstantBuffers(slot, 1, &buffer); break;
	case ShaderStage::HS: graphics->GetDeviceContext()->HSSetConstantBuffers(slot, 1, &buffer); break;
	case ShaderStage::DS: graphics->GetDeviceContext()->DSSetConstantBuffers(slot, 1, &buffer); break;
	case ShaderStage::GS: graphics->GetDeviceContext()->GSSetConstantBuffers(slot, 1, &buffer); break;
	case ShaderStage::CS: graphics->GetDeviceContext()->CSSetConstantBuffers(slot, 1, &buffer); break;
	}
}


#pragma region oldVersion
//#include "Framework.h"
//#include "ConstantBuffer.h"
//
//ConstantBuffer::ConstantBuffer(Context * context)
//	: buffer(nullptr)
//{
//	graphics = context->GetSubsystem<Graphics>();
//}
//
//ConstantBuffer::~ConstantBuffer()
//{
//	Clear();
//}
//
//void * ConstantBuffer::Map()
//{
//	D3D11_MAPPED_SUBRESOURCE mappedResource;
//	auto hr = graphics->GetDeviceContext()->Map
//	(
//		buffer,
//		0,
//		D3D11_MAP_WRITE_DISCARD,
//		0,
//		&mappedResource
//	);
//	assert(SUCCEEDED(hr));
//
//	return mappedResource.pData;
//}
//
//void ConstantBuffer::Unmap()
//{
//	graphics->GetDeviceContext()->Unmap(buffer, 0);
//}
//
//void ConstantBuffer::Clear()
//{
//	SAFE_RELEASE(buffer);
//}
//
//void ConstantBuffer::BindPipeline(const ShaderType & type, const uint & slot)
//{
//	switch (type)
//	{
//	case ShaderType::VS: graphics->GetDeviceContext()->VSSetConstantBuffers(slot, 1, &buffer); break;
//	case ShaderType::PS: graphics->GetDeviceContext()->PSSetConstantBuffers(slot, 1, &buffer); break;
//	case ShaderType::HS: graphics->GetDeviceContext()->HSSetConstantBuffers(slot, 1, &buffer); break;
//	case ShaderType::DS: graphics->GetDeviceContext()->DSSetConstantBuffers(slot, 1, &buffer); break;
//	case ShaderType::GS: graphics->GetDeviceContext()->GSSetConstantBuffers(slot, 1, &buffer); break;
//	case ShaderType::CS: graphics->GetDeviceContext()->CSSetConstantBuffers(slot, 1, &buffer); break;
//	}
//}
#pragma endregion