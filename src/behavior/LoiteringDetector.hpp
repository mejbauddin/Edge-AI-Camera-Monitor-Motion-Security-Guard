#pragma once

#include "BehaviorSettings.hpp"
#include "types/Frame.hpp"

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace csx::behavior {

class LoiteringDetector {
public:
    explicit LoiteringDetector(BehaviorSettings settings);

    void detect(const std::vector<core::Track>& tracks, std::chrono::steady_clock::time_point now,
                std::vector<core::BehaviorAnomaly>& outAnomalies);
    void pruneInactive(const std::vector<core::Track>& tracks);

private:
    struct DwellState {
        core::Point2f anchor{};
        std::chrono::steady_clock::time_point since{};
        bool active{false};
    };

    BehaviorSettings settings_;
    std::unordered_map<std::uint32_t, DwellState> dwellByTrack_;
};

}  // namespace csx::behavior
