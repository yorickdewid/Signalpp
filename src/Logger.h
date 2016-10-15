#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>

#define SIGNAL_LOG_CRITICAL   \
	if (signalpp::Logger::getCurrentLogLevel() <= signalpp::LogLevel::CRITICAL) \
		signalpp::Logger("CRITICAL", signalpp::LogLevel::CRITICAL)
#define SIGNAL_LOG_ERROR      \
	if (signalpp::Logger::getCurrentLogLevel() <= signalpp::LogLevel::ERROR) \
		signalpp::Logger("ERROR   ", signalpp::LogLevel::ERROR)
#define SIGNAL_LOG_WARNING    \
	if (signalpp::Logger::getCurrentLogLevel() <= signalpp::LogLevel::WARNING) \
		signalpp::Logger("WARNING ", signalpp::LogLevel::WARNING)
#define SIGNAL_LOG_INFO       \
	if (signalpp::Logger::getCurrentLogLevel() <= signalpp::LogLevel::INFO) \
		signalpp::Logger("INFO    ", signalpp::LogLevel::INFO)
#define SIGNAL_LOG_DEBUG      \
	if (signalpp::Logger::getCurrentLogLevel() <= signalpp::LogLevel::DEBUG) \
		signalpp::Logger("DEBUG   ", signalpp::LogLevel::DEBUG)

namespace signalpp {

enum class LogLevel {
	DEBUG = 0,
	INFO,
	WARNING,
	ERROR,
	CRITICAL,
};

class LogHandlerContainer {
  public:
	virtual void log(std::string message, LogLevel level) = 0;
};

class CoutLogHandler : public LogHandlerContainer {
  public:
	void log(std::string message, LogLevel /*level*/) override {
		std::cout << message;
	}
};

class Logger {
	LogLevel m_level;
	std::ostringstream m_stringstream;

	static std::string getTimestamp() {
		char date[32];
		time_t t = time(0);
		tm _tm;

#ifdef _MSC_VER
		gmtime_s(&_tm, &t);
#else
		gmtime_r(&t, &_tm);
#endif

		size_t sz = strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &_tm);
		return std::string(date, date+sz);
	}

  public:
	Logger(const std::string prefix, LogLevel level) : m_level(level) {
#ifdef ENABLE_LOGGING
		m_stringstream << "(" << getTimestamp() << ") [" << prefix << "] ";
#endif
	}

	~Logger() {
#ifdef ENABLE_LOGGING
		if (m_level >= getCurrentLogLevel()) {
			m_stringstream << std::endl;
			getHandler()->log(m_stringstream.str(), m_level);
		}
#endif
	}

	template <typename T>
	Logger& operator<<(T const &value) {
#ifdef ENABLE_LOGGING
		if (m_level >= getCurrentLogLevel()) {
			m_stringstream << value;
		}
#endif
		return *this;
	}

	static void setLogLevel(LogLevel level) {
		getLogLevel() = level;
	}

	static void setHandler(LogHandlerContainer* handler) {
		getHandler() = handler;
	}

	static LogLevel getCurrentLogLevel() {
		return getLogLevel();
	}

  private:
	static LogLevel& getLogLevel() {
		static LogLevel current_level = LogLevel::ERROR;
		return current_level;
	}

	static LogHandlerContainer*& getHandler() {
		static CoutLogHandler default_handler;
		static LogHandlerContainer* current_handler = &default_handler;
		return current_handler;
	}
};

} // namespace signalpp

#endif // _LOGGER_H_
