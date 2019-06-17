#pragma once

enum class ComponentType : uint
{
	Unknown,
	Camera,
	Skybox,
	Transform,
	Terrain,
	Ocean,
	Light,
	Renderable
};

class IComponent
{
public:
	template <typename T>
	static constexpr ComponentType DeduceComponentType();

public:
	IComponent(class Context * context,class Actor * actor,class Transform * transform);
	virtual ~IComponent() = default;

	virtual void Serialize(FileStream* stream) = 0;
	virtual void Deserialize(FileStream* stream) = 0;

	virtual void OnInitialize() = 0;
	virtual void OnStart() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnStop() = 0;
	virtual void OnDestroy() = 0;

	const uint& GetID() const { return id; }
	class Actor* GetActor() const { return actor; }
	class Transform* GetTransform() const { return transform; }
	const ComponentType& GetComponentType() const { return componentType; }
	const bool& IsEnabled() const { return bEnabled; }

	void SetComponentType(const ComponentType& type) { this->componentType = type; }
	void SetIsEnabled(const bool& bEnabled) { this->bEnabled = bEnabled; }

protected:
	class Context* context;
	class Actor* actor;
	class Transform* transform;
	ComponentType componentType;
	bool bEnabled;

private:
	uint id;
};
