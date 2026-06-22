#pragma once

#include "interfaces/Interfaces.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>

namespace csx::core {

class Logger final : public ILogger {
public:
    explicit Logger(std::string logDirectory = "logs");

    void trace(const std::string& module, const std::string& message) override;
    void debug(const std::string& module, const std::string& message) override;
    void info(const std::string& module, const std::string& message) override;
    void warn(const std::string& module, const std::string& message) override;
    void error(const std::string& module, const std::string& message) override;
    void aiDecision(const std::string& module, const std::string& message) override;
    void performance(const std::string& module, const std::string& message) override;

    void setLevel(const std::string& level);

private:
    void write(const std::string& level, const std::string& module, const std::string& message,
               const std::string& fileSuffix = "");

    std::filesystem::path logDirectory_;
    std::mutex mutex_;
    std::string level_{"info"};
};

std::shared_ptr<ILogger> createLogger(const std::string& logDirectory = "logs");

}  // namespace csx::core
