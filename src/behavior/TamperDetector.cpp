#include "TamperDetector.hpp"

#include <cmath>

namespace csx::behavior {

TamperDetector::TamperDetector(BehaviorSettings settings, BaselineLearner* baseline)
    : settings_(std::move(settings)), baseline_(baseline) {}

void TamperDetector::updateSettings(const BehaviorSettings& settings) {
    settings_ = settings;
}

void TamperDetector::analyze(const core::Frame& frame, const std::vector<core::Track>& tracks,
                             const FrameStats& stats, const std::chrono::steady_clock::time_point now,
                             std::vector<core::BehaviorAnomaly>& outAnomalies) {
    (void)frame;

    if (baseline_ != nullptr && baseline_->ready()) {
        const float brightnessDelta = std::abs(stats.brightness - baseline_->meanBrightness());
        if (brightnessDelta >= settings_.tamperBrightnessDelta) {
            core::BehaviorAnomaly anomaly;
            anomaly.type = core::BehaviorAnomalyType::CameraTampering;
            anomaly.score = settings_.tamperScore;
            anomaly.description = "Sudden brightness shift suggests camera tampering";
            outAnomalies.push_back(std::move(anomaly));
        }

        if (baseline_->meanEdgeEnergy() > 1e-4F) {
            const float edgeRatio = stats.edgeEnergy / baseline_->meanEdgeEnergy();
            if (edgeRatio <= settings_.tamperBlurDropRatio) {
                core::BehaviorAnomaly anomaly;
                anomaly.type = core::BehaviorAnomalyType::CameraTampering;
                anomaly.score = settings_.tamperScore * 0.95F;
                anomaly.description = "Edge energy collapse suggests lens obstruction or blur";
                outAnomalies.push_back(std::move(anomaly));
            }
        }
    }

    if (hasFrameStats_) {
        const float brightnessJump = std::abs(stats.brightness - lastBrightness_);
        const float edgeDrop = lastEdgeEnergy_ > 1e-4F ? stats.edgeEnergy / lastEdgeEnergy_ : 1.0F;
        if (brightnessJump > settings_.tamperBrightnessDelta * 0.75F && edgeDrop < 0.6F) {
            core::BehaviorAnomaly anomaly;
            anomaly.type = core::BehaviorAnomalyType::CameraTampering;
            anomaly.score = settings_.tamperScore;
            anomaly.description = "Frame statistics indicate possible camera cover or disconnect";
            outAnomalies.push_back(std::move(anomaly));
        }
    }

    lastBrightness_ = stats.brightness;
    lastEdgeEnergy_ = stats.edgeEnergy;
    hasFrameStats_ = true;

    const bool sceneIdle = tracks.empty();
    if (sceneIdle) {
        if (!idleSince_) {
            idleSince_ = now;
        } else {
            const auto idleMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(now - *idleSince_);
            if (idleMs.count() >= static_cast<std::int64_t>(settings_.idleCameraMs)) {
                core::BehaviorAnomaly anomaly;
                anomaly.type = core::BehaviorAnomalyType::IdleCamera;
                anomaly.score = settings_.idleCameraScore;
                anomaly.duration = idleMs;
                anomaly.description = "No motion activity detected for extended period";
                outAnomalies.push_back(std::move(anomaly));
            }
        }
    } else {
        idleSince_.reset();
    }
}

}  // namespace csx::behavior
