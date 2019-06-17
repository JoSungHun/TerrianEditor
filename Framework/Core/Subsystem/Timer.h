#pragma once
#include "ISubsystem.h"

class Timer final : public ISubsystem
{
public:
	Timer(class Context* context);
	~Timer() = default;

	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	const bool Initialize() override;

	auto GetDeltaTimeMs() const -> const float& { return deltaTimeMs; }
	auto GetDeltaTimeSec() const -> const float& { return deltaTimeSec; }

	void Update();

private:
	float deltaTimeMs;
	float deltaTimeSec;

	std::chrono::high_resolution_clock::time_point prevTime;
};