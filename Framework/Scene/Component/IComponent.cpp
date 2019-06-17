#include "Framework.h"
#include "IComponent.h"
#include "Camera.h"
#include "Transform.h"
#include "SkyBox\SkyBox.h"
#include "Renderable.h"
#include "Terrain\Terrain.h"
#include "Ocean\Ocean.h"


IComponent::IComponent(Context * context, Actor * actor, Transform * transform)
	: context(context)
	, actor(actor)
	, transform(transform)
	, bEnabled(true)
{
	id = GUIDGenerator::Generate();
}

template <typename T>
constexpr ComponentType IComponent::DeduceComponentType()
{
	return ComponentType::Unknown;
}

#define REGISTER_COMPONENT(T, enumT) template<> ComponentType IComponent::DeduceComponentType<T>() { return enumT; }

REGISTER_COMPONENT(Camera, ComponentType::Camera)
REGISTER_COMPONENT(Transform, ComponentType::Transform)
REGISTER_COMPONENT(Renderable, ComponentType::Renderable)
REGISTER_COMPONENT(Light, ComponentType::Light)
REGISTER_COMPONENT(Skybox, ComponentType::Skybox)
REGISTER_COMPONENT(Terrain, ComponentType::Terrain)
REGISTER_COMPONENT(Ocean, ComponentType::Ocean)