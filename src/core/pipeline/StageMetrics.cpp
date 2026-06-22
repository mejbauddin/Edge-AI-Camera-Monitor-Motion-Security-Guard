#include <algorithm>

#include "pipeline/StageMetrics.hpp"

namespace csx::core {

StageMetrics::StageMetrics(const std::size_t windowSize) : windowSize_(windowSize == 0 ? 1 : windowSize) {}

void StageMetrics::record(const std::string& stageName, const std::chrono::microseconds duration) {
    const double ms = static_cast<double>(duration.count()) / 1000.0;
    std::lock_guard lock(mutex_);
    auto& window = stages_[stageName];
    window.samples.push_back(ms);
    window.sumMs += ms;
    if (window.samples.size() > windowSize_) {
        window.sumMs -= window.samples.front();
        window.samples.pop_front();
    }
}

double StageMetrics::averageMs(const std::string& stageName) const {
    std::lock_guard lock(mutex_);
    const auto it = stages_.find(stageName);
    if (it == stages_.end() || it->second.samples.empty()) {
        return 0.0;
    }
    return it->second.sumMs / static_cast<double>(it->second.samples.size());
}

double StageMetrics::maxMs(const std::string& stageName) const {
    std::lock_guard lock(mutex_);
    const auto it = stages_.find(stageName);
    if (it == stages_.end() || it->second.samples.empty()) {
        return 0.0;
    }
    double maxValue = 0.0;
    for (const double sample : it->second.samples) {
        maxValue = std::max(maxValue, sample);
    }
    return maxValue;
}

std::size_t StageMetrics::sampleCount(const std::string& stageName) const {
    std::lock_guard lock(mutex_);
    const auto it = stages_.find(stageName);
    return it == stages_.end() ? 0 : it->second.samples.size();
}

}  // namespace csx::core
