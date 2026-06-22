#include "BehaviorEngine.hpp"

#include "Timer.hpp"

namespace csx::behavior {

BehaviorEngine::BehaviorEngine(BehaviorSettings settings)
    : settings_(std::move(settings)),
      baseline_(settings_),
      intrusionDetector_(settings_),
      loiteringDetector_(settings_),
      motionAnalyzer_(settings_, &baseline_),
      tamperDetector_(settings_, &baseline_),
      scorer_(settings_) {}

void BehaviorEngine::setSettings(const BehaviorSettings& settings) {
    settings_ = settings;
    baseline_ = BaselineLearner(settings_);
    intrusionDetector_ = IntrusionDetector(settings_);
    loiteringDetector_ = LoiteringDetector(settings_);
    motionAnalyzer_.updateSettings(settings_);
    tamperDetector_.updateSettings(settings_);
    scorer_ = AnomalyScorer(settings_);
}

const BehaviorSettings& BehaviorEngine::settings() const noexcept {
    return settings_;
}

BaselineLearner& BehaviorEngine::baseline() noexcept {
    return baseline_;
}

void BehaviorEngine::analyze(const core::Frame& frame, const std::vector<core::Track>& tracks,
                             std::vector<core::BehaviorAnomaly>& outAnomalies) {
    outAnomalies.clear();

    if (!settings_.enabled) {
        health_.status = core::EngineStatus::Degraded;
        health_.detail = "Behavior analysis disabled";
        return;
    }

    csx::utils::Timer timer;
    const auto now = std::chrono::steady_clock::now();
    const FrameStats stats = analyzeFrame(frame);

    baseline_.observe(frame, tracks, stats);

    std::vector<core::BehaviorAnomaly> rawAnomalies;
    intrusionDetector_.detect(tracks, rawAnomalies);
    loiteringDetector_.detect(tracks, now, rawAnomalies);
    loiteringDetector_.pruneInactive(tracks);
    motionAnalyzer_.analyze(frame, tracks, rawAnomalies);
    tamperDetector_.analyze(frame, tracks, stats, now, rawAnomalies);

    outAnomalies = std::move(rawAnomalies);
    scorer_.finalize(outAnomalies);

    ++processedFrames_;
    lastLatencyMs_ = timer.elapsedMs();
    health_.status = baseline_.ready() ? core::EngineStatus::Online : core::EngineStatus::Starting;
    health_.confidence = baseline_.ready() ? 1.0F : static_cast<float>(baseline_.observedFrames()) /
                                                     static_cast<float>(settings_.baselineLearningFrames);
    health_.lastLatencyMs = static_cast<float>(lastLatencyMs_);
    health_.processedFrames = processedFrames_;
    health_.detail = baseline_.ready() ? "Baseline learned" : "Learning normal behavior";
}

core::EngineHealth BehaviorEngine::health() const {
    return health_;
}

std::shared_ptr<core::IBehaviorEngine> createBehaviorEngine(BehaviorSettings settings) {
    return std::make_shared<BehaviorEngine>(std::move(settings));
}

}  // namespace csx::behavior
