#include "Framework.h"
#include "InstanceBuffer.h"

InstanceBuffer::InstanceBuffer(Context * context)
	:buffer(nullptr)
	,stride(0)
	,offset(0)
	, instanceCount(0)
{
	graphics = context->GetSubsystem<Graphics>();
}

InstanceBuffer::~InstanceBuffer()
{
	Clear();
}

void InstanceBuffer::Clear()
{
	stride = 0;
	offset = 0;
	instanceCount = 0;

	SAFE_RELEASE(buffer);
}

