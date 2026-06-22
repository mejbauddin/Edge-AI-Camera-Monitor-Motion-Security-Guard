#include "BehaviorFusion.hpp"

#include "types/Enums.hpp"

#include <algorithm>
#include <sstream>

namespace csx::threat {

BehaviorFusion::BehaviorFusion(ThreatSettings settings) : settings_(std::move(settings)) {}

void BehaviorFusion::setSettings(const ThreatSettings& settings) {
    settings_ = settings;
}

float BehaviorFusion::typeWeight(const core::BehaviorAnomalyType type) const {
    switch (type) {
        case core::BehaviorAnomalyType::Intrusion:
        case core::BehaviorAnomalyType::CameraTampering:
            return 1.0F;
        case core::BehaviorAnomalyType::Loitering:
        case core::BehaviorAnomalyType::NightActivity:
            return 0.85F;
        case core::BehaviorAnomalyType::Running:
        case core::BehaviorAnomalyType::Falling:
        case core::BehaviorAnomalyType::UnusualMovement:
            return 0.70F;
        case core::BehaviorAnomalyType::UnexpectedObject:
        case core::BehaviorAnomalyType::UnexpectedDisappearance:
            return 0.60F;
        case core::BehaviorAnomalyType::IdleCamera:
            return 0.30F;
        case core::BehaviorAnomalyType::None:
            return 0.0F;
    }
    return 0.0F;
}

BehaviorFusionResult BehaviorFusion::fuse(const std::vector<core::BehaviorAnomaly>& anomalies) const {
    BehaviorFusionResult result;
    if (anomalies.empty()) {
        return result;
    }

    float weightedSum = 0.0F;
    float weightTotal = 0.0F;
    std::ostringstream summary;

    for (const auto& anomaly : anomalies) {
        const float weight = typeWeight(anomaly.type);
        weightedSum += anomaly.score * weight * 100.0F;
        weightTotal += weight;
        if (!summary.str().empty()) {
            summary << "; ";
        }
        summary << core::toString(anomaly.type);
    }

    result.behaviorScore =
        weightTotal > 0.0F ? std::clamp(weightedSum / weightTotal, 0.0F, 100.0F) : 0.0F;
    result.normalizedScore = result.behaviorScore / 100.0F;
    result.summary = summary.str();
    return result;
}

}  // namespace csx::threat
