#pragma once

enum EngineFlags : ulong
{
	EngineFlags_Update = 1UL << 0,
	EngineFlags_Render = 1UL << 1,
	EngineFlags_Game = 1UL << 2,
};

//0x00000001
//0x00000010
//0x00000100

//0x00000101
//0x00000010

class Engine final
{
public:
	static auto GetEngineFlags()->const ulong& { return engineFlags; }
	static void SetEngineFlags(const ulong& flags) { engineFlags = flags; }
	static void OnEngineFlag(const EngineFlags& flag) { engineFlags |= flag; }
	static void OffEngineFlag(const EngineFlags& flag) { engineFlags &= ~flag; }
	static void ToggleEngineFlag(const EngineFlags& flag)
	{
		IsOnEngineFlag(flag) ? OffEngineFlag(flag) : OnEngineFlag(flag);
	}
	static auto IsOnEngineFlag(const EngineFlags& flag)->const bool
	{
		return (engineFlags & flag) > 0UL;
	}

public:
	Engine();
	~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	auto GetContext() const -> class Context* { return context; }

	void Update();

private:
	static ulong engineFlags;

	class Context* context;
};