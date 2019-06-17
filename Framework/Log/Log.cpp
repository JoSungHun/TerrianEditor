#include "Framework.h"
#include "Log.h"
#include "ILogger.h"

std::string     Log::caller;
ILogger*        Log::logger;
std::ofstream   Log::out;
std::string     Log::fileName;
std::mutex      Log::logMutex;
bool            Log::bFirstLog;

void Log::SetLogger(ILogger * iLogger)
{
	logger = iLogger;
}

void Log::Write(const char * text, const LogType & type)
{
	auto formattedText = caller.empty() ? std::string(text) : caller + ":" + std::string(text);

	logger ? LogToLogger(formattedText.c_str(), type) : LogToFile(formattedText.c_str(), type);

	caller.clear();
}

void Log::Write(const std::string & text, const LogType & type)
{
	Write(text.c_str(), type);
}

void Log::InfoF(const char * text, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, sizeof(buffer), text, args);
	va_end(args);

	Write(buffer, LogType::Info);
}

void Log::WarningF(const char * text, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, sizeof(buffer), text, args);
	va_end(args);

	Write(buffer, LogType::Warning);
}

void Log::ErrorF(const char * text, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, text);
	vsnprintf(buffer, sizeof(buffer), text, args);
	va_end(args);

	Write(buffer, LogType::Error);
}

void Log::Write(const bool & val, const LogType & type)
{
	val ? Write("True", type) : Write("False", type);
}

void Log::LogToLogger(const char * text, const LogType & type)
{
	std::lock_guard<std::mutex> guard(logMutex);
	logger->Log(std::string(text), static_cast<int>(type));
}

void Log::LogToFile(const char * text, const LogType & type)
{
	std::lock_guard<std::mutex> guared(logMutex);

	std::string prefix = (type == LogType::Info) ? "Info : " : (type == LogType::Warning) ? "Warning : " : "Error : ";
	std::string finalText = prefix + text;

	out.open("log.txt", std::ios::out | std::ios::app);
	out << finalText << std::endl;
	out.close();
}
