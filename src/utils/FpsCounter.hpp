#pragma once

#include <chrono>
#include <cstddef>
#include <deque>

namespace csx::utils {

class FpsCounter {
public:
    explicit FpsCounter(std::size_t windowSize = 60);

    void tick(std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now());
    [[nodiscard]] double fps() const;
    [[nodiscard]] double frameTimeMs() const;
    [[nodiscard]] std::size_t sampleCount() const;

    void reset();

private:
    std::size_t windowSize_;
    std::deque<std::chrono::steady_clock::time_point> samples_;
};

}  // namespace csx::utils
