#pragma once

enum class ResourceType : uint
{
	Unknown,
	Texture,
	Material,
	Mesh
};

class IResource
{
public:
	template <typename T>
	static constexpr ResourceType DeduceResourceType();

public:
	IResource(class Context* context);
	virtual ~IResource() = default;


//====================================================================================
//	GETTER
//====================================================================================
	const uint& GetResourceID()				const { return resourceID; }
	const ResourceType& GetResourceType()	const { return resourceType; }
	const std::string& GetResourceName()	const { return resourceName; }
	const std::string& GetResourcePath()	const { return resourcePath; }
	
//====================================================================================
//	SETTER
//====================================================================================
	void SetResourceType(const ResourceType& type)	{ this->resourceType = type; }
	void SetResourceName(const std::string& name)	{ this->resourceName = name; }
	void SetResourcePath(const std::string& path)	{ this->resourcePath = path; }

	
//====================================================================================
//	SAVE & LOAD
//====================================================================================
	virtual void LoadFromFile(const std::string& path) = 0;
	virtual void SaveToFile(const std::string& path) = 0;

protected:
	class Context* context;
	class ResourceManager* resourceManager;

	uint resourceID;
	ResourceType resourceType;
	std::string resourceName;
	std::string resourcePath;
};