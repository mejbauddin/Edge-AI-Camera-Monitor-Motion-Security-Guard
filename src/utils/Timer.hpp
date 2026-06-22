#pragma once

#include <chrono>

namespace csx::utils {

class Timer {
public:
    Timer();

    void reset();
    [[nodiscard]] std::chrono::nanoseconds elapsed() const;
    [[nodiscard]] double elapsedMs() const;
    [[nodiscard]] double elapsedUs() const;

private:
    std::chrono::steady_clock::time_point start_;
};

class ScopedTimer {
public:
    explicit ScopedTimer(double& outMilliseconds);
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    Timer timer_;
    double& outMilliseconds_;
};

}  // namespace csx::utils
