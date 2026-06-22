#pragma once

#include "BaselineLearner.hpp"
#include "BehaviorSettings.hpp"
#include "types/Frame.hpp"

#include <chrono>
#include <optional>
#include <vector>

namespace csx::behavior {

class TamperDetector {
public:
    TamperDetector(BehaviorSettings settings, BaselineLearner* baseline);

    void updateSettings(const BehaviorSettings& settings);
    void analyze(const core::Frame& frame, const std::vector<core::Track>& tracks,
                 const FrameStats& stats, std::chrono::steady_clock::time_point now,
                 std::vector<core::BehaviorAnomaly>& outAnomalies);

private:
    BehaviorSettings settings_;
    BaselineLearner* baseline_{nullptr};
    std::optional<std::chrono::steady_clock::time_point> idleSince_;
    float lastBrightness_{0.0F};
    float lastEdgeEnergy_{0.0F};
    bool hasFrameStats_{false};
};

}  // namespace csx::behavior
