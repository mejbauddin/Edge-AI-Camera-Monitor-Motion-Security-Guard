#include "SweepController.hpp"

#include <cmath>

namespace csx::radar {

SweepController::SweepController(float rpm) 
    : rpm_(rpm), lastUpdateTime_(std::chrono::steady_clock::now()) {}

void SweepController::update() {
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration<float>(now - lastUpdateTime_).count();
    lastUpdateTime_ = now;

    const float currentRpm = rpm_.load(std::memory_order_relaxed);
    const float radiansPerSecond = currentRpm * 2.0F * 3.14159265F / 60.0F;
    const float deltaAngle = radiansPerSecond * elapsed;

    float angle = currentAngle_.load(std::memory_order_relaxed);
    angle = std::fmod(angle + deltaAngle, 2.0F * 3.14159265F);
    currentAngle_.store(angle, std::memory_order_relaxed);
}

void SweepController::setRpm(float rpm) noexcept {
    rpm_.store(std::max(0.1F, rpm), std::memory_order_relaxed);
}

}  // namespace csx::radar
