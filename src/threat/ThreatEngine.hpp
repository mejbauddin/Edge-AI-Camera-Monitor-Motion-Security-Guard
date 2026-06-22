#pragma once

#include "BehaviorFusion.hpp"
#include "PriorityCalculator.hpp"
#include "ThreatHistory.hpp"
#include "ThreatScorer.hpp"
#include "ThreatSettings.hpp"
#include "interfaces/Interfaces.hpp"

#include <memory>

namespace csx::threat {

class ThreatEngine final : public core::IThreatEngine {
public:
    explicit ThreatEngine(ThreatSettings settings = {});

    core::ThreatAssessment assess(const std::vector<core::Track>& tracks,
                                  const std::vector<core::FaceMatch>& faces,
                                  const std::vector<core::BehaviorAnomaly>& anomalies) override;
    [[nodiscard]] core::EngineHealth health() const override;

    void setSettings(const ThreatSettings& settings);
    [[nodiscard]] const ThreatSettings& settings() const noexcept;
    [[nodiscard]] ThreatHistory& history() noexcept;
    [[nodiscard]] bool shouldAutoRecord(core::ThreatLevel level) const;

private:
    [[nodiscard]] bool isNightHour() const;

    ThreatSettings settings_;
    BehaviorFusion behaviorFusion_;
    ThreatScorer scorer_;
    PriorityCalculator priorityCalculator_;
    ThreatHistory history_;
    core::EngineHealth health_;
    std::uint64_t assessments_{0};
    double lastLatencyMs_{0.0};
};

std::shared_ptr<core::IThreatEngine> createThreatEngine(ThreatSettings settings = {});

}  // namespace csx::threat
