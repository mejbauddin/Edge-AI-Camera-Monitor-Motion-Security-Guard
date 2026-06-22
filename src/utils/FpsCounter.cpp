#include "FpsCounter.hpp"

#include <algorithm>

namespace csx::utils {

FpsCounter::FpsCounter(const std::size_t windowSize) : windowSize_(windowSize == 0 ? 1 : windowSize) {}

void FpsCounter::tick(const std::chrono::steady_clock::time_point timestamp) {
    samples_.push_back(timestamp);
    while (samples_.size() > windowSize_) {
        samples_.pop_front();
    }
}

double FpsCounter::fps() const {
    if (samples_.size() < 2) {
        return 0.0;
    }

    const auto duration = samples_.back() - samples_.front();
    const auto count = static_cast<double>(samples_.size() - 1);
    const auto seconds = std::chrono::duration<double>(duration).count();
    if (seconds <= 0.0) {
        return 0.0;
    }
    return count / seconds;
}

double FpsCounter::frameTimeMs() const {
    const auto currentFps = fps();
    return currentFps > 0.0 ? 1000.0 / currentFps : 0.0;
}

std::size_t FpsCounter::sampleCount() const {
    return samples_.size();
}

void FpsCounter::reset() {
    samples_.clear();
}

}  // namespace csx::utils
