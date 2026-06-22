#include "ThreatEngine.hpp"

#include "Timer.hpp"
#include "types/Enums.hpp"

#include <ctime>

namespace csx::threat {

ThreatEngine::ThreatEngine(ThreatSettings settings)
    : settings_(std::move(settings)),
      behaviorFusion_(settings_),
      scorer_(settings_),
      priorityCalculator_(settings_),
      history_(settings_) {}

void ThreatEngine::setSettings(const ThreatSettings& settings) {
    settings_ = settings;
    behaviorFusion_.setSettings(settings_);
    scorer_.setSettings(settings_);
    priorityCalculator_.setSettings(settings_);
    history_.setSettings(settings_);
}

const ThreatSettings& ThreatEngine::settings() const noexcept {
    return settings_;
}

ThreatHistory& ThreatEngine::history() noexcept {
    return history_;
}

bool ThreatEngine::isNightHour() const {
    const std::time_t raw = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
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

bool ThreatEngine::shouldAutoRecord(const core::ThreatLevel level) const {
    return static_cast<int>(level) >= static_cast<int>(settings_.autoRecordLevel);
}

core::ThreatAssessment ThreatEngine::assess(const std::vector<core::Track>& tracks,
                                            const std::vector<core::FaceMatch>& faces,
                                            const std::vector<core::BehaviorAnomaly>& anomalies) {
    csx::utils::Timer timer;

    core::ThreatAssessment assessment;
    assessment.timestamp = std::chrono::system_clock::now();

    if (!settings_.enabled) {
        health_.status = core::EngineStatus::Degraded;
        health_.detail = "Threat engine disabled";
        return assessment;
    }

    const auto behavior = behaviorFusion_.fuse(anomalies);
    const auto signals = scorer_.compose(tracks, faces, behavior.behaviorScore, isNightHour());
    const float smoothedScore = history_.smoothScore(signals.rawTotal);

    assessment.threatScore = smoothedScore;
    assessment.behaviorScore = behavior.behaviorScore;
    assessment.level = core::threatLevelFromScore(smoothedScore, settings_.levels.greenMax,
                                                settings_.levels.yellowMax, settings_.levels.orangeMax,
                                                settings_.levels.redMax);
    assessment.defcon = core::defconFromThreatLevel(assessment.level);

    const auto priority = priorityCalculator_.calculate(
        assessment.level, assessment.threatScore, assessment.behaviorScore, tracks, faces, anomalies,
        behavior.summary);
    assessment.priority = priority.priority;
    assessment.severity = priority.severity;
    assessment.confidence = priority.confidence;
    assessment.decisionReason = priority.decisionReason;
    assessment.contributingTrackIds = std::move(priority.contributingTrackIds);

    history_.record(assessment);

    ++assessments_;
    lastLatencyMs_ = timer.elapsedMs();
    health_.status = core::EngineStatus::Online;
    health_.confidence = assessment.confidence;
    health_.lastLatencyMs = static_cast<float>(lastLatencyMs_);
    health_.processedFrames = assessments_;
    health_.detail = core::toString(assessment.level) + " @ " +
                     std::to_string(static_cast<int>(assessment.threatScore));

    return assessment;
}

core::EngineHealth ThreatEngine::health() const {
    return health_;
}

std::shared_ptr<core::IThreatEngine> createThreatEngine(ThreatSettings settings) {
    return std::make_shared<ThreatEngine>(std::move(settings));
}

}  // namespace csx::threat
