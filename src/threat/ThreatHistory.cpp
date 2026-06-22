#include "ThreatHistory.hpp"

#include <algorithm>

namespace csx::threat {

ThreatHistory::ThreatHistory(ThreatSettings settings) : settings_(std::move(settings)) {}

void ThreatHistory::setSettings(const ThreatSettings& settings) {
    settings_ = settings;
    while (history_.size() > settings_.historyCapacity) {
        history_.pop_front();
    }
}

float ThreatHistory::smoothScore(const float rawScore) {
    if (!initialized_) {
        smoothedScore_ = rawScore;
        initialized_ = true;
        return smoothedScore_;
    }

    if (rawScore >= smoothedScore_) {
        smoothedScore_ = rawScore;
    } else {
        smoothedScore_ = smoothedScore_ * (1.0F - settings_.deescalationRate) +
                         rawScore * settings_.deescalationRate;
    }

    return std::clamp(smoothedScore_, 0.0F, 100.0F);
}

core::ThreatLevel ThreatHistory::peakLevel() const noexcept {
    return peakLevel_;
}

std::size_t ThreatHistory::size() const noexcept {
    return history_.size();
}

void ThreatHistory::record(const core::ThreatAssessment& assessment) {
    history_.push_back(assessment);
    while (history_.size() > settings_.historyCapacity) {
        history_.pop_front();
    }

    if (static_cast<int>(assessment.level) > static_cast<int>(peakLevel_)) {
        peakLevel_ = assessment.level;
    } else if (assessment.level == core::ThreatLevel::Green &&
               assessment.threatScore <= settings_.levels.greenMax) {
        peakLevel_ = core::ThreatLevel::Green;
    }
}

void ThreatHistory::clear() {
    history_.clear();
    smoothedScore_ = 0.0F;
    peakLevel_ = core::ThreatLevel::Green;
    initialized_ = false;
}

}  // namespace csx::threat
