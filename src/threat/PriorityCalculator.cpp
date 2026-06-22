#include "PriorityCalculator.hpp"

#include "types/Enums.hpp"

#include <algorithm>
#include <sstream>

namespace csx::threat {

PriorityCalculator::PriorityCalculator(ThreatSettings settings) : settings_(std::move(settings)) {}

void PriorityCalculator::setSettings(const ThreatSettings& settings) {
    settings_ = settings;
}

PriorityResult PriorityCalculator::calculate(const core::ThreatLevel level, const float threatScore,
                                             const float behaviorScore,
                                             const std::vector<core::Track>& tracks,
                                             const std::vector<core::FaceMatch>& faces,
                                             const std::vector<core::BehaviorAnomaly>& anomalies,
                                             const std::string& behaviorSummary) const {
    PriorityResult result;
    result.contributingTrackIds.reserve(tracks.size() + faces.size());

    for (const auto& track : tracks) {
        if (track.active) {
            result.contributingTrackIds.push_back(track.id);
        }
    }
    for (const auto& face : faces) {
        if (face.trackId != 0 &&
            std::find(result.contributingTrackIds.begin(), result.contributingTrackIds.end(),
                      face.trackId) == result.contributingTrackIds.end()) {
            result.contributingTrackIds.push_back(face.trackId);
        }
    }

    int basePriority = 0;
    int baseSeverity = 0;
    switch (level) {
        case core::ThreatLevel::Green:
            basePriority = 1;
            baseSeverity = 1;
            break;
        case core::ThreatLevel::Yellow:
            basePriority = 3;
            baseSeverity = 3;
            break;
        case core::ThreatLevel::Orange:
            basePriority = 5;
            baseSeverity = 5;
            break;
        case core::ThreatLevel::Red:
            basePriority = 7;
            baseSeverity = 7;
            break;
        case core::ThreatLevel::Critical:
            basePriority = 10;
            baseSeverity = 10;
            break;
    }

    const bool foePresent = std::any_of(
        faces.begin(), faces.end(),
        [](const core::FaceMatch& face) {
            return face.classification == core::IdentityClassification::Foe;
        });
    const bool intrusionPresent = std::any_of(
        anomalies.begin(), anomalies.end(), [](const core::BehaviorAnomaly& anomaly) {
            return anomaly.type == core::BehaviorAnomalyType::Intrusion ||
                   anomaly.type == core::BehaviorAnomalyType::CameraTampering;
        });

    if (foePresent) {
        basePriority += 2;
        baseSeverity += 2;
    }
    if (intrusionPresent) {
        basePriority += 1;
        baseSeverity += 1;
    }

    result.priority = std::clamp(basePriority, 1, 10);
    result.severity = std::clamp(baseSeverity, 1, 10);
    result.confidence = std::clamp(0.4F + threatScore / 200.0F + behaviorScore / 300.0F, 0.0F, 1.0F);

    std::ostringstream reason;
    reason << core::toString(level) << " threat (score=" << static_cast<int>(threatScore) << ")";
    if (!behaviorSummary.empty()) {
        reason << " — behavior: " << behaviorSummary;
    }
    if (foePresent) {
        reason << " — foe identity detected";
    }
    result.decisionReason = reason.str();
    return result;
}

}  // namespace csx::threat
