#pragma once

#include "BaselineLearner.hpp"
#include "BehaviorSettings.hpp"
#include "types/Frame.hpp"

#include <chrono>
#include <vector>

namespace csx::behavior {

class MotionPatternAnalyzer {
public:
    MotionPatternAnalyzer(BehaviorSettings settings, BaselineLearner* baseline);

    void updateSettings(const BehaviorSettings& settings);
    void analyze(const core::Frame& frame, const std::vector<core::Track>& tracks,
                 std::vector<core::BehaviorAnomaly>& outAnomalies);

private:
    [[nodiscard]] bool isNightHour(std::chrono::system_clock::time_point timestamp) const;

    BehaviorSettings settings_;
    BaselineLearner* baseline_{nullptr};
};

}  // namespace csx::behavior
