#pragma once

#include <chrono>
#include <cstdint>

namespace csx::tracking {

struct TrackingSettings {
    std::uint32_t maxTracks{32};
    std::uint32_t lostTimeoutMs{1500};
    float iouThreshold{0.3F};
    std::uint32_t historyLength{30};
    float velocitySmoothing{0.6F};
};

}  // namespace csx::tracking
