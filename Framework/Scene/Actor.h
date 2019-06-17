#pragma once
#include "./Component/IComponent.h"
#include "./Component/Transform.h"

class Actor final
{
public:
	Actor(class Context* context);
	~Actor();

	Actor(const Actor&) = delete;
	Actor& operator=(const Actor&) = delete;

	void Serialize(FileStream* stream);
	void Deserialize(FileStream* stream, Transform* transform);

	void Initialize(class Transform* transform);
	void Start();
	void Update();
	void Stop();

	auto GetID() const -> const uint& { return id; }

	const std::string& GetName() const { return name; }
	void SetName(const std::string& name) { this->name = name; }

	const bool& IsActive() const { return bActive; }
	void SetIsActive(const bool& bActive) { this->bActive = bActive; }

	auto GetTransform() const -> class Transform* { return transform; }

	template <typename T> T* AddComponent();
	template <typename T> T* GetComponent();
	template <typename T> std::vector<T*> GetComponents();

	const bool HasComponent(const ComponentType& type);

	template <typename T>
	const bool HasComponent()
	{
		return HasComponent(IComponent::DeduceComponentType<T>());
	}

	template <typename T>
	void RemoveComponent();

private:
	class Context* context;
	class Transform* transform;
	std::string name;
	uint id;
	bool bActive;

	std::vector<IComponent*> components;
};

template<typename T>
inline T * Actor::AddComponent()
{
	ComponentType type = IComponent::DeduceComponentType<T>();

	if (HasComponent(type))
		return GetComponent<T>();

	components.emplace_back
	(
		new T
		(
			context,
			this,
			transform
		)
	);

	auto newComponent = static_cast<T*>(components.back());
	newComponent->SetComponentType(type);
	newComponent->OnInitialize();

	return newComponent;
}

template<typename T>
inline T * Actor::GetComponent()
{
	ComponentType type = IComponent::DeduceComponentType<T>();

	for (auto component : components)
	{
		if (component->GetComponentType() == type)
			return static_cast<T*>(component);
	}
	return nullptr;
}

template<typename T>
inline std::vector<T*> Actor::GetComponents()
{
	ComponentType type = IComponent::DeduceComponentType<T>();

	std::vector<T*> tempComponents;
	for (auto component : components)
	{
		if (component->GetComponentType() != type)
			continue;

		tempComponents.emplace_back(static_cast<T*>(component));
	}

	return tempComponents;
}

template<typename T>
inline void Actor::RemoveComponent()
{
	ComponentType type = IComponent::DeduceComponentType<T>();

	for (auto iter = components.begin(); iter != components.end();)
	{
		auto component = *iter;
		if (component->GetComponentType() == type)
		{
			component->OnDestroy();
			SAFE_DELETE(component);
			iter = components.erase(iter);
		}
		else
			iter++;
	}
}
