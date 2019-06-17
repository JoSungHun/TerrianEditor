#pragma once

enum class LogType : uint
{
	Info,
	Warning,
	Error,
};

#define LOG_INFO(text)              { Log::caller = __FUNCTION__; Log::Write(text, LogType::Info); }
#define LOG_WARNING(text)           { Log::caller = __FUNCTION__; Log::Write(text, LogType::Warning); }
#define LOG_ERROR(text)             { Log::caller = __FUNCTION__; Log::Write(text, LogType::Error); }
#define LOG_INFO_F(text, ...)       { Log::caller = __FUNCTION__; Log::InfoF(text, __VA_ARGS__); }
#define LOG_WARNING_F(text, ...)    { Log::caller = __FUNCTION__; Log::WarningF(text, __VA_ARGS__); }
#define LOG_ERROR_F(text, ...)      { Log::caller = __FUNCTION__; Log::ErrorF(text, __VA_ARGS__); }

class Log final
{
public:
	static void SetLogger(class ILogger* iLogger);

	static void Write(const char* text, const LogType& type);
	static void Write(const std::string& text, const LogType& type);

	static void InfoF(const char* text, ...);
	static void WarningF(const char* text, ...);
	static void ErrorF(const char* text, ...);

	template <typename T, typename = typename std::enable_if<
		std::is_same<T, int>::value ||
		std::is_same<T, uint>::value ||
		std::is_same<T, long>::value ||
		std::is_same<T, long long>::value ||
		std::is_same<T, unsigned long>::value ||
		std::is_same<T, float>::value ||
		std::is_same<T, double>::value ||
		std::is_same<T, long double>::value
	>::type>
		static void Write(const T& val, const LogType& type)
	{
		Write(std::to_string(val), type);
	}

	static void Write(const bool& val, const LogType& type);

	static void LogToLogger(const char* text, const LogType& type);
	static void LogToFile(const char* text, const LogType& type);

public:
	static std::string caller;

private:
	static class ILogger* logger;
	static std::ofstream out;
	static std::string fileName;
	static std::mutex logMutex;
	static bool bFirstLog;

private:
	Log() = default;
	~Log() = default;

	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
};