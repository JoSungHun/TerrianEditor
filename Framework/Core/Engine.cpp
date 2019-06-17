#include "Framework.h"
#include "Engine.h"

ulong Engine::engineFlags = 0UL;

Engine::Engine()
{
	engineFlags = EngineFlags_Update | EngineFlags_Render;

	context = new Context();
	context->RegisterSubsystem<Timer>();
	context->RegisterSubsystem<Input>();
	context->RegisterSubsystem<Thread>();
	context->RegisterSubsystem<Graphics>();
	context->RegisterSubsystem<ResourceManager>();
	context->RegisterSubsystem<SceneManager>();
	context->RegisterSubsystem<Renderer>();

	context->InitializeSubsystems();
}

Engine::~Engine()
{
	SAFE_DELETE(context);
}

void Engine::Update()
{
	if (IsOnEngineFlag(EngineFlags_Update))
		EventSystem::Get().Fire(EventType::Update);

	if (IsOnEngineFlag(EngineFlags_Render))
		EventSystem::Get().Fire(EventType::Render);
}