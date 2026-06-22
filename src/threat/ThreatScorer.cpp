#include "ThreatScorer.hpp"

#include <algorithm>
#include <cmath>

namespace csx::threat {

namespace {

float velocityMagnitude(const core::Point2f& velocity) {
    return std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
}

}  // namespace

ThreatScorer::ThreatScorer(ThreatSettings settings) : settings_(std::move(settings)) {}

void ThreatScorer::setSettings(const ThreatSettings& settings) {
    settings_ = settings;
}

float ThreatScorer::scoreMotion(const std::vector<core::Track>& tracks) const {
    if (tracks.empty()) {
        return 0.0F;
    }

    float totalVelocity = 0.0F;
    std::size_t activeCount = 0;
    for (const auto& track : tracks) {
        if (!track.active) {
            continue;
        }
        ++activeCount;
        totalVelocity += velocityMagnitude(track.velocity);
    }

    if (activeCount == 0) {
        return 0.0F;
    }

    const float avgVelocity = totalVelocity / static_cast<float>(activeCount);
    const float countFactor = std::min(static_cast<float>(activeCount) * 12.0F, 40.0F);
    const float velocityFactor = std::min(avgVelocity * 0.8F, 60.0F);
    return std::clamp(countFactor + velocityFactor, 0.0F, 100.0F);
}

float ThreatScorer::scoreTrackActivity(const std::vector<core::Track>& tracks) const {
    if (tracks.empty()) {
        return 0.0F;
    }

    float confidenceSum = 0.0F;
    std::size_t activeCount = 0;
    for (const auto& track : tracks) {
        if (!track.active) {
            continue;
        }
        ++activeCount;
        confidenceSum += track.confidence;
    }

    if (activeCount == 0) {
        return 0.0F;
    }

    const float avgConfidence = confidenceSum / static_cast<float>(activeCount);
    const float countScore = std::min(static_cast<float>(activeCount) * 18.0F, 72.0F);
    return std::clamp(countScore * (0.5F + avgConfidence * 0.5F), 0.0F, 100.0F);
}

float ThreatScorer::scoreFaces(const std::vector<core::FaceMatch>& faces) const {
    if (faces.empty()) {
        return 0.0F;
    }

    float highest = 0.0F;
    bool hasAuthorized = false;

    for (const auto& face : faces) {
        switch (face.classification) {
            case core::IdentityClassification::Foe:
                highest = std::max(highest, settings_.foeFaceScore);
                break;
            case core::IdentityClassification::Unknown:
                highest = std::max(highest, settings_.unknownFaceScore * face.confidence);
                break;
            case core::IdentityClassification::Authorized:
                hasAuthorized = true;
                highest = std::max(highest, settings_.authorizedFaceScore);
                break;
        }
    }

    if (hasAuthorized && highest <= settings_.authorizedFaceScore + 1.0F) {
        highest = std::max(0.0F, highest - settings_.authorizedFaceReduction);
    }

    return std::clamp(highest, 0.0F, 100.0F);
}

float ThreatScorer::scoreContext(const bool nightHours) const {
    return nightHours ? settings_.nightContextBoost : 0.0F;
}

SignalScores ThreatScorer::compose(const std::vector<core::Track>& tracks,
                                   const std::vector<core::FaceMatch>& faces,
                                   const float behaviorScore, const bool nightHours) const {
    SignalScores scores;
    scores.motion = scoreMotion(tracks);
    scores.track = scoreTrackActivity(tracks);
    scores.face = scoreFaces(faces);
    scores.behavior = std::clamp(behaviorScore, 0.0F, 100.0F);
    scores.context = scoreContext(nightHours);

    const auto& w = settings_.weights;
    scores.rawTotal = scores.motion * w.motion + scores.track * w.track + scores.face * w.face +
                      scores.behavior * w.behavior + scores.context * w.context;
    scores.rawTotal = std::clamp(scores.rawTotal, 0.0F, 100.0F);
    return scores;
}

}  // namespace csx::threat
