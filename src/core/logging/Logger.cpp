#include "logging/Logger.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace csx::core {

namespace {

std::string timestampNow() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif
    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

int levelRank(const std::string& level) {
    if (level == "trace") return 0;
    if (level == "debug") return 1;
    if (level == "info")  return 2;
    if (level == "warn")  return 3;
    if (level == "error") return 4;
    return 2;
}

}  // namespace

Logger::Logger(std::string logDirectory) : logDirectory_(std::move(logDirectory)) {
    std::filesystem::create_directories(logDirectory_);
}

void Logger::setLevel(const std::string& level) {
    std::lock_guard lock(mutex_);
    level_ = level;
}

void Logger::write(const std::string& level, const std::string& module,
                   const std::string& message, const std::string& fileSuffix) {
    std::lock_guard lock(mutex_);
    if (levelRank(level) < levelRank(level_)) {
        return;
    }

    const auto line = "[" + timestampNow() + "] [" + level + "] [" + module + "] " + message;
    std::cout << line << '\n';

    const auto date = timestampNow().substr(0, 10);
    const auto filePath = logDirectory_ / ("cyber_sentinel_" + date + fileSuffix + ".log");
    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        file << line << '\n';
    }
}

void Logger::trace(const std::string& module, const std::string& message) {
    write("trace", module, message);
}

void Logger::debug(const std::string& module, const std::string& message) {
    write("debug", module, message);
}

void Logger::info(const std::string& module, const std::string& message) {
    write("info", module, message);
}

void Logger::warn(const std::string& module, const std::string& message) {
    write("warn", module, message);
}

void Logger::error(const std::string& module, const std::string& message) {
    write("error", module, message);
}

void Logger::aiDecision(const std::string& module, const std::string& message) {
    write("info", module, message, "_ai");
}

void Logger::performance(const std::string& module, const std::string& message) {
    write("debug", module, message, "_perf");
}

std::shared_ptr<ILogger> createLogger(const std::string& logDirectory) {
    return std::make_shared<Logger>(logDirectory);
}

}  // namespace csx::core
