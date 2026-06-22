#include "IntrusionDetector.hpp"

#include "Geometry.hpp"

namespace csx::behavior {

IntrusionDetector::IntrusionDetector(BehaviorSettings settings)
    : settings_(std::move(settings)) {}

void IntrusionDetector::detect(const std::vector<core::Track>& tracks,
                               std::vector<core::BehaviorAnomaly>& outAnomalies) const {
    for (const auto& track : tracks) {
        if (!track.active) {
            continue;
        }

        for (const auto& zone : settings_.zones) {
            if (!zone.restricted || !pointInPolygon(track.center, zone.polygon)) {
                continue;
            }

            core::BehaviorAnomaly anomaly;
            anomaly.type = core::BehaviorAnomalyType::Intrusion;
            anomaly.score = settings_.intrusionScore;
            anomaly.region = track.bbox;
            anomaly.trackId = track.id;
            anomaly.description = "Track entered restricted zone '" + zone.id + "'";
            outAnomalies.push_back(std::move(anomaly));
            break;
        }
    }
}

}  // namespace csx::behavior
