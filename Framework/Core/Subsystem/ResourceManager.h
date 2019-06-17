#pragma once
#include "ISubsystem.h"
#include "../../Resource/IResource.h"

class ResourceManager final : public ISubsystem
{
public:
	ResourceManager(class Context* context);
	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(ResourceManager&) = delete;

	const bool Initialize() override;

	auto GetModelImporter() const -> class ModelImporter* { return modelImporter; }
	auto GetImageImporter() const -> class ImageImporter* { return imageImporter; }

	auto GetProjectDirectory() const -> const std::string& { return projectDirectory; }
	void SetProjectDirectory(const std::string& directory);

	template <typename T> T* Load(const std::string& path);
	template <typename T> T* GetResourceFromName(const std::string& name);
	template <typename T> T* GetResourceFromPath(const std::string& path);
	template <typename T> const std::string GetResourceDirectory();

	void RegisterResource(class IResource* resource);
	void RegisterResourceDirectory(const ResourceType& type, const std::string& directory);
	void RegisterResourceInDirectory(const ResourceType& type);

private:
	class ModelImporter* modelImporter;
	class ImageImporter* imageImporter;

	std::string projectDirectory;

	typedef std::vector<class IResource*> ResourceGroup;
	std::map<ResourceType, ResourceGroup> resourceGroups;
	std::map<ResourceType, std::string> resourceDirectories;
};

template<typename T>
inline T * ResourceManager::Load(const std::string & path)
{
	auto resource = GetResourceFromPath<T>(path);

	if (!resource)
	{
		auto type = IResource::DeduceResourceType<T>();
		auto directory = resourceDirectories[type];

		resource = new T(context);
		resource->SetResourceType(type);
		resource->SetResourceName(FileSystem::GetIntactFileNameFromPath(path));
		resource->SetResourcePath(path);
		resource->LoadFromFile(path);

		RegisterResource(resource);
	}

	return resource;
}

template<typename T>
inline T * ResourceManager::GetResourceFromName(const std::string & name)
{
	for (auto resource : resourceGroups[IResource::DeduceResourceType<T>()])
	{
		if (resource->GetResourceName() == name)
			return static_cast<T*>(resource);
	}
	return nullptr;
}

template<typename T>
inline T * ResourceManager::GetResourceFromPath(const std::string & path)
{
	for (auto resource : resourceGroups[IResource::DeduceResourceType<T>()])
	{
		if (resource->GetResourcePath() == path)
			return static_cast<T*>(resource);
	}
	return nullptr;
}

template<typename T>
inline const std::string ResourceManager::GetResourceDirectory()
{
	return resourceDirectories[IResource::DeduceResourceType<T>()];
}
