#pragma once
#include "IWidget.h"
#include "./Log/EngineLogger.h"

class Widget_Log final : public IWidget
{
public:
	Widget_Log(class Context* context);
	~Widget_Log();

	Widget_Log(const Widget_Log&) = delete;
	Widget_Log& operator=(const Widget_Log&) = delete;

	void Render() override;

	void AddLog(const LogPair& logPair);
	void Clear();

private:
	EngineLogger* logger;
	std::deque<LogPair> logs;
	uint maxLogCount;
	bool bShowInfo;
	bool bShowWarning;
	bool bShowError;
	bool bScroll;
};