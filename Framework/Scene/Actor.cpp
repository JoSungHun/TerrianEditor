#include "Framework.h"
#include "Actor.h"

Actor::Actor(Context * context)
	: context(context)
	, transform(nullptr)
	, name("")
	, bActive(true)
{
	id = GUIDGenerator::Generate();
}

Actor::~Actor()
{
	for (auto iter = components.begin(); iter != components.end(); iter++)
	{
		(*iter)->OnDestroy();
		SAFE_DELETE(*iter);
	}

	components.clear();
}

void Actor::Serialize(FileStream * stream)
{
}

void Actor::Deserialize(FileStream * stream, Transform * transform)
{
}

void Actor::Initialize(Transform * transform)
{
	this->transform = transform;
}

void Actor::Start()
{
	if (!bActive)
		return;

	for (auto component : components)
		component->OnStart();
}

void Actor::Update()
{
	if (!bActive)
		return;

	for (auto component : components)
		component->OnUpdate();
}

void Actor::Stop()
{
	if (!bActive)
		return;

	for (auto component : components)
		component->OnStop();
}

const bool Actor::HasComponent(const ComponentType & type)
{
	for (auto component : components)
	{
		if (component->GetComponentType() == type)
			return true;
	}
	return false;
}
