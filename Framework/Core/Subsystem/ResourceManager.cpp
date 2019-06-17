#include "Framework.h"
#include "ResourceManager.h"
#include "Resource/Importer/ImageImporter.h"

ResourceManager::ResourceManager(Context * context)
	: ISubsystem(context)
	, modelImporter(nullptr)
	, imageImporter(nullptr)
{
}

ResourceManager::~ResourceManager()
{
	for (auto resourceGroup : resourceGroups)
	{
		for (auto resource : resourceGroup.second)
			SAFE_DELETE(resource);
	}

	resourceGroups.clear();
	resourceDirectories.clear();

	SAFE_DELETE(imageImporter);
	SAFE_DELETE(modelImporter);
}

const bool ResourceManager::Initialize()
{
	imageImporter = new ImageImporter(context);

	SetProjectDirectory("../_Project/");

	RegisterResourceDirectory(ResourceType::Texture, "../_Assets/Texture/");
	RegisterResourceDirectory(ResourceType::Material, "../_Assets/Material/");

	return true;
}

void ResourceManager::SetProjectDirectory(const std::string & directory)
{
	if (!FileSystem::ExistDirectory(directory))
		FileSystem::Create_Directory(directory);

	projectDirectory = directory;
}

void ResourceManager::RegisterResource(IResource * resource)
{
	resourceGroups[resource->GetResourceType()].emplace_back(resource);
}

void ResourceManager::RegisterResourceDirectory(const ResourceType & type, const std::string & directory)
{
	resourceDirectories[type] = directory;
	RegisterResourceInDirectory(type);
}

void ResourceManager::RegisterResourceInDirectory(const ResourceType & type)
{
	auto path = resourceDirectories[type];
	if (!FileSystem::IsDirectory(path))
		return;
	
	
	auto files = FileSystem::GetFilesInDirectory(path);
	for (const auto file : files)
	{
		switch (type)
		{
		case ResourceType::Texture:		Load<Texture>(file);	break;
		case ResourceType::Material:	Load<Material>(file);	break;
		case ResourceType::Mesh:		Load<Mesh>(file);		break;
		}
	}

}
