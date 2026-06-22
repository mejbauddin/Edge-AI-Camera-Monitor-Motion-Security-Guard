#include "LoiteringDetector.hpp"

#include "Geometry.hpp"

#include <algorithm>

namespace csx::behavior {

LoiteringDetector::LoiteringDetector(BehaviorSettings settings)
    : settings_(std::move(settings)) {}

void LoiteringDetector::detect(const std::vector<core::Track>& tracks,
                               const std::chrono::steady_clock::time_point now,
                               std::vector<core::BehaviorAnomaly>& outAnomalies) {
    for (const auto& track : tracks) {
        if (!track.active) {
            continue;
        }

        const float speed = velocityMagnitude(track.velocity);
        auto& dwell = dwellByTrack_[track.id];

        if (!dwell.active || distance(track.center, dwell.anchor) > settings_.loiteringRadiusPx) {
            dwell.anchor = track.center;
            dwell.since = now;
            dwell.active = true;
            continue;
        }

        if (speed > settings_.loiteringMaxSpeed) {
            continue;
        }

        const auto elapsedMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - dwell.since);
        if (elapsedMs.count() < static_cast<std::int64_t>(settings_.loiteringThresholdMs)) {
            continue;
        }

        core::BehaviorAnomaly anomaly;
        anomaly.type = core::BehaviorAnomalyType::Loitering;
        anomaly.score = settings_.loiteringScore;
        anomaly.region = track.bbox;
        anomaly.trackId = track.id;
        anomaly.duration = elapsedMs;
        anomaly.description = "Track dwelling in area for " + std::to_string(elapsedMs.count()) + " ms";
        outAnomalies.push_back(std::move(anomaly));
    }
}

void LoiteringDetector::pruneInactive(const std::vector<core::Track>& tracks) {
    for (auto it = dwellByTrack_.begin(); it != dwellByTrack_.end();) {
        const bool stillActive =
            std::any_of(tracks.begin(), tracks.end(),
                        [&](const core::Track& track) { return track.id == it->first && track.active; });
        if (!stillActive) {
            it = dwellByTrack_.erase(it);
        } else {
            ++it;
        }
    }
}

}  // namespace csx::behavior
