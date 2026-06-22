#pragma once

#include "AnomalyScorer.hpp"
#include "BaselineLearner.hpp"
#include "BehaviorSettings.hpp"
#include "IntrusionDetector.hpp"
#include "LoiteringDetector.hpp"
#include "MotionPatternAnalyzer.hpp"
#include "TamperDetector.hpp"
#include "interfaces/Interfaces.hpp"

#include <memory>

namespace csx::behavior {

class BehaviorEngine final : public core::IBehaviorEngine {
public:
    explicit BehaviorEngine(BehaviorSettings settings = {});

    void analyze(const core::Frame& frame, const std::vector<core::Track>& tracks,
                 std::vector<core::BehaviorAnomaly>& outAnomalies) override;
    [[nodiscard]] core::EngineHealth health() const override;

    void setSettings(const BehaviorSettings& settings);
    [[nodiscard]] const BehaviorSettings& settings() const noexcept;
    [[nodiscard]] BaselineLearner& baseline() noexcept;

private:
    BehaviorSettings settings_;
    BaselineLearner baseline_;
    IntrusionDetector intrusionDetector_;
    LoiteringDetector loiteringDetector_;
    MotionPatternAnalyzer motionAnalyzer_;
    TamperDetector tamperDetector_;
    AnomalyScorer scorer_;
    core::EngineHealth health_;
    std::uint64_t processedFrames_{0};
    double lastLatencyMs_{0.0};
};

std::shared_ptr<core::IBehaviorEngine> createBehaviorEngine(BehaviorSettings settings = {});

}  // namespace csx::behavior
