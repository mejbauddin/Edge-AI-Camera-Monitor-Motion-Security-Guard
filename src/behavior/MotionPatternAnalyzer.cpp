#include "MotionPatternAnalyzer.hpp"

#include "Geometry.hpp"

#include <ctime>

namespace csx::behavior {

MotionPatternAnalyzer::MotionPatternAnalyzer(BehaviorSettings settings, BaselineLearner* baseline)
    : settings_(std::move(settings)), baseline_(baseline) {}

void MotionPatternAnalyzer::updateSettings(const BehaviorSettings& settings) {
    settings_ = settings;
}

bool MotionPatternAnalyzer::isNightHour(const std::chrono::system_clock::time_point timestamp) const {
    const std::time_t raw = std::chrono::system_clock::to_time_t(timestamp);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &raw);
#else
    localtime_r(&raw, &localTime);
#endif
    const auto hour = static_cast<std::uint32_t>(localTime.tm_hour);
    if (settings_.nightHourStart <= settings_.nightHourEnd) {
        return hour >= settings_.nightHourStart && hour < settings_.nightHourEnd;
    }
    return hour >= settings_.nightHourStart || hour < settings_.nightHourEnd;
}

void MotionPatternAnalyzer::analyze(const core::Frame& frame, const std::vector<core::Track>& tracks,
                                    std::vector<core::BehaviorAnomaly>& outAnomalies) {
    const auto now = std::chrono::system_clock::now();
    const bool nightActivity = isNightHour(now) && !tracks.empty();

    if (nightActivity && baseline_ != nullptr && baseline_->ready()) {
        core::BehaviorAnomaly anomaly;
        anomaly.type = core::BehaviorAnomalyType::NightActivity;
        anomaly.score = settings_.nightActivityScore;
        anomaly.description = "Activity detected during configured night hours";
        outAnomalies.push_back(std::move(anomaly));
    }

    if (baseline_ != nullptr && baseline_->ready() && baseline_->isUnusualTrackCount(tracks.size()) &&
        !tracks.empty()) {
        core::BehaviorAnomaly anomaly;
        anomaly.type = core::BehaviorAnomalyType::UnexpectedObject;
        anomaly.score = settings_.unusualMovementScore;
        anomaly.description = "Unexpected object count relative to learned baseline";
        outAnomalies.push_back(std::move(anomaly));
    }

    for (const auto& track : tracks) {
        if (!track.active) {
            continue;
        }

        const float speed = velocityMagnitude(track.velocity);

        if (speed >= settings_.runningSpeedThreshold) {
            core::BehaviorAnomaly anomaly;
            anomaly.type = core::BehaviorAnomalyType::Running;
            anomaly.score = settings_.runningScore;
            anomaly.region = track.bbox;
            anomaly.trackId = track.id;
            anomaly.description = "High-velocity movement detected";
            outAnomalies.push_back(std::move(anomaly));
        }

        if (track.acceleration.y <= settings_.fallingAccelerationThreshold) {
            core::BehaviorAnomaly anomaly;
            anomaly.type = core::BehaviorAnomalyType::Falling;
            anomaly.score = settings_.runningScore;
            anomaly.region = track.bbox;
            anomaly.trackId = track.id;
            anomaly.description = "Sudden downward acceleration detected";
            outAnomalies.push_back(std::move(anomaly));
        }

        if (baseline_ != nullptr && baseline_->ready() && baseline_->isUnusualVelocity(speed)) {
            core::BehaviorAnomaly anomaly;
            anomaly.type = core::BehaviorAnomalyType::UnusualMovement;
            anomaly.score = settings_.unusualMovementScore;
            anomaly.region = track.bbox;
            anomaly.trackId = track.id;
            anomaly.description = "Movement deviates from learned baseline";
            outAnomalies.push_back(std::move(anomaly));
        }
    }

    if (baseline_ != nullptr && baseline_->ready() && tracks.empty() &&
        frame.sequence > baseline_->observedFrames()) {
        core::BehaviorAnomaly anomaly;
        anomaly.type = core::BehaviorAnomalyType::UnexpectedDisappearance;
        anomaly.score = settings_.unusualMovementScore * 0.5F;
        anomaly.description = "Activity dropped below learned baseline";
        outAnomalies.push_back(std::move(anomaly));
    }
}

}  // namespace csx::behavior
