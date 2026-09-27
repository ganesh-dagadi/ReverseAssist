#include "Logging.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

Logger& Logger::getInstance() {
	static Logger logger;
	return logger;
}

void Logger::setConsoleSinkEnabled(bool enabled) {
	std::lock_guard<std::mutex> lock(mutex_);
	consoleSinkEnabled_ = enabled;
}

bool Logger::setFileSink(const std::string& path) {
	std::lock_guard<std::mutex> lock(mutex_);
	if (file_.is_open()) {
		file_.close();
	}

	file_.clear();
	file_.open(path, std::ios::out | std::ios::app);
	return file_.is_open();
}

void Logger::disableFileSink() {
	std::lock_guard<std::mutex> lock(mutex_);
	if (file_.is_open()) {
		file_.close();
	}
}

void Logger::log(Level level, const std::string& message) {
	const std::string formattedMessage = formatMessage(level, message);
	std::lock_guard<std::mutex> lock(mutex_);
	if (consoleSinkEnabled_) {
		writeToConsole(formattedMessage);
	}
	writeToFile(formattedMessage);
}

void Logger::debug(const std::string& message) {
	log(Level::Debug, message);
}

void Logger::info(const std::string& message) {
	log(Level::Info, message);
}

void Logger::warning(const std::string& message) {
	log(Level::Warning, message);
}

void Logger::error(const std::string& message) {
	log(Level::Error, message);
}

const char* Logger::levelName(Level level) {
	switch (level) {
	case Level::Debug:
		return "DEBUG";
	case Level::Info:
		return "INFO";
	case Level::Warning:
		return "WARNING";
	case Level::Error:
		return "ERROR";
	}
	return "UNKNOWN";
}

std::string Logger::formatMessage(Level level, const std::string& message) {
	const auto now = std::chrono::system_clock::now();
	const auto time = std::chrono::system_clock::to_time_t(now);
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()) % 1000;
	std::tm localTime{};
	localtime_r(&time, &localTime);

	std::ostringstream formatted;
	formatted << '[' << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
		<< '.' << std::setfill('0') << std::setw(3) << milliseconds.count()
		<< "] [" << levelName(level) << "] " << message;
	return formatted.str();
}

void Logger::writeToConsole(const std::string& formattedMessage) {
	std::clog << formattedMessage << '\n';
}

void Logger::writeToFile(const std::string& formattedMessage) {
	if (file_.is_open()) {
		file_ << formattedMessage << '\n';
		file_.flush();
	}
}