#pragma once

#include <fstream>
#include <mutex>
#include <string>

class Logger {
public:
	enum class Level {
		Debug,
		Info,
		Warning,
		Error
	};

	static Logger& getInstance();

	void setConsoleSinkEnabled(bool enabled);
	bool setFileSink(const std::string& path);
	void disableFileSink();

	void log(Level level, const std::string& message);
	void debug(const std::string& message);
	void info(const std::string& message);
	void warning(const std::string& message);
	void error(const std::string& message);

private:
	Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	static const char* levelName(Level level);
	static std::string formatMessage(Level level, const std::string& message);
	void writeToConsole(const std::string& formattedMessage);
	void writeToFile(const std::string& formattedMessage);

	std::mutex mutex_;
	std::ofstream file_;
	bool consoleSinkEnabled_ = true;
};

