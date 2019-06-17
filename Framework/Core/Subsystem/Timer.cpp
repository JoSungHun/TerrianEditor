#include "Framework.h"
#include "Timer.h"

Timer::Timer(Context * context)
	: ISubsystem(context)
	, deltaTimeMs(0.0f)
	, deltaTimeSec(0.0f)
{
	EventSystem::Get().Subscribe(EventType::Update, EVENT_HANDLER(Update));
}

const bool Timer::Initialize()
{
	prevTime = std::chrono::high_resolution_clock::now();
	return true;
}

void Timer::Update()
{
	auto curTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> ms = curTime - prevTime;
	prevTime = curTime;

	deltaTimeMs = static_cast<float>(ms.count());
	deltaTimeSec = static_cast<float>(ms.count() * 0.001);
}
