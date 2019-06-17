#pragma once
#include "Framework.h"

class ILogger
{
public:
	virtual ~ILogger() = default;
	virtual void Log(const std::string& log, const int& type) = 0;
};