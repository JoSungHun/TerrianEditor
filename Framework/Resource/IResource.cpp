#include "Framework.h"
#include "IResource.h"

IResource::IResource(Context* context)
	: context(context)
{
	resourceID = GUIDGenerator::Generate();
	resourceManager = context->GetSubsystem<class ResourceManager>();
}

template <typename T>
constexpr ResourceType IResource::DeduceResourceType()
{
	return ResourceType::Unknown;
}

#define REGISTER_RESOURCE_TYPE(T, enumT) template<> ResourceType IResource::DeduceResourceType<T>() { return enumT; }

REGISTER_RESOURCE_TYPE(Texture, ResourceType::Texture)
REGISTER_RESOURCE_TYPE(Material, ResourceType::Material)
REGISTER_RESOURCE_TYPE(Mesh, ResourceType::Mesh)