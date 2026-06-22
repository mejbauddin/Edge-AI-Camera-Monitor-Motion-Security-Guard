#pragma once

#include <chrono>
#include <cstddef>
#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

namespace csx::core {

class StageMetrics {
public:
    explicit StageMetrics(std::size_t windowSize = 120);

    void record(const std::string& stageName, std::chrono::microseconds duration);
    [[nodiscard]] double averageMs(const std::string& stageName) const;
    [[nodiscard]] double maxMs(const std::string& stageName) const;
    [[nodiscard]] std::size_t sampleCount(const std::string& stageName) const;

private:
    struct StageWindow {
        std::deque<double> samples;
        double sumMs{0.0};
    };

    mutable std::mutex mutex_;
    std::size_t windowSize_;
    std::unordered_map<std::string, StageWindow> stages_;
};

}  // namespace csx::core
