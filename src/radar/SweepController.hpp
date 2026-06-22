#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace csx::radar {

// ──────────────────────────────────────────────────────────────────────────────
// SweepController — manages radar sweep animation state
// ──────────────────────────────────────────────────────────────────────────────
class SweepController {
public:
    explicit SweepController(float rpm = 4.0F);
    ~SweepController() = default;

    // Update sweep angle based on elapsed time
    void update();

    // Get current sweep angle in radians (0 at right, clockwise)
    [[nodiscard]] float currentAngle() const noexcept { return currentAngle_.load(std::memory_order_relaxed); }

    // Set sweep speed in RPM
    void setRpm(float rpm) noexcept;

    [[nodiscard]] float rpm() const noexcept { return rpm_.load(std::memory_order_relaxed); }

    // Reset sweep to starting position
    void reset() noexcept { currentAngle_.store(0.0F, std::memory_order_relaxed); }

private:
    std::atomic<float> currentAngle_{0.0F};  // radians
    std::atomic<float> rpm_{4.0F};           // revolutions per minute
    
    std::chrono::steady_clock::time_point lastUpdateTime_;
};

}  // namespace csx::radar
