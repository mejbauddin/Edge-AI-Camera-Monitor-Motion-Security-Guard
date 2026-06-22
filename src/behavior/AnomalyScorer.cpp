#include "AnomalyScorer.hpp"

#include <algorithm>
#include <unordered_map>

namespace csx::behavior {

AnomalyScorer::AnomalyScorer(BehaviorSettings settings) : settings_(std::move(settings)) {}

void AnomalyScorer::finalize(std::vector<core::BehaviorAnomaly>& anomalies) const {
    std::unordered_map<std::uint64_t, core::BehaviorAnomaly> bestByKey;
    bestByKey.reserve(anomalies.size());

    for (auto& anomaly : anomalies) {
        anomaly.score = std::clamp(anomaly.score, 0.0F, 1.0F);
        if (anomaly.score < settings_.minAnomalyScore) {
            continue;
        }

        const std::uint64_t key =
            (static_cast<std::uint64_t>(anomaly.type) << 32U) |
            static_cast<std::uint64_t>(anomaly.trackId);
        const auto found = bestByKey.find(key);
        if (found == bestByKey.end() || anomaly.score > found->second.score) {
            bestByKey[key] = std::move(anomaly);
        }
    }

    anomalies.clear();
    anomalies.reserve(bestByKey.size());
    for (auto& [_, anomaly] : bestByKey) {
        anomalies.push_back(std::move(anomaly));
    }

    std::sort(anomalies.begin(), anomalies.end(),
              [](const core::BehaviorAnomaly& lhs, const core::BehaviorAnomaly& rhs) {
                  return lhs.score > rhs.score;
              });
}

}  // namespace csx::behavior
