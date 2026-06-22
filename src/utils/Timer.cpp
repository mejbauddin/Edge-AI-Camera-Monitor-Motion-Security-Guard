#include "Timer.hpp"

namespace csx::utils {

Timer::Timer() : start_(std::chrono::steady_clock::now()) {}

void Timer::reset() {
    start_ = std::chrono::steady_clock::now();
}

std::chrono::nanoseconds Timer::elapsed() const {
    return std::chrono::steady_clock::now() - start_;
}

double Timer::elapsedMs() const {
    return static_cast<double>(elapsed().count()) / 1'000'000.0;
}

double Timer::elapsedUs() const {
    return static_cast<double>(elapsed().count()) / 1'000.0;
}

ScopedTimer::ScopedTimer(double& outMilliseconds) : outMilliseconds_(outMilliseconds) {}

ScopedTimer::~ScopedTimer() {
    outMilliseconds_ = timer_.elapsedMs();
}

}  // namespace csx::utils
