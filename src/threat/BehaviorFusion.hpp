#pragma once

#include "ThreatSettings.hpp"
#include "types/Frame.hpp"

#include <vector>

namespace csx::threat {

struct BehaviorFusionResult {
    float behaviorScore{0.0F};
    float normalizedScore{0.0F};
    std::string summary;
};

class BehaviorFusion {
public:
    explicit BehaviorFusion(ThreatSettings settings = {});

    [[nodiscard]] BehaviorFusionResult fuse(const std::vector<core::BehaviorAnomaly>& anomalies) const;
    void setSettings(const ThreatSettings& settings);

private:
    [[nodiscard]] float typeWeight(core::BehaviorAnomalyType type) const;

    ThreatSettings settings_;
};

}  // namespace csx::threat
