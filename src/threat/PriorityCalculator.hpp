#pragma once

#include "ThreatSettings.hpp"
#include "types/Frame.hpp"

#include <string>
#include <vector>

namespace csx::threat {

struct PriorityResult {
    int priority{0};
    int severity{0};
    std::string decisionReason;
    std::vector<std::uint32_t> contributingTrackIds;
    float confidence{0.0F};
};

class PriorityCalculator {
public:
    explicit PriorityCalculator(ThreatSettings settings = {});

    [[nodiscard]] PriorityResult calculate(core::ThreatLevel level, float threatScore,
                                           float behaviorScore,
                                           const std::vector<core::Track>& tracks,
                                           const std::vector<core::FaceMatch>& faces,
                                           const std::vector<core::BehaviorAnomaly>& anomalies,
                                           const std::string& behaviorSummary) const;

    void setSettings(const ThreatSettings& settings);

private:
    ThreatSettings settings_;
};

}  // namespace csx::threat
