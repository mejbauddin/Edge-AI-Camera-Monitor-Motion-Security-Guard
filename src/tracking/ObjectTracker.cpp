#include "ObjectTracker.hpp"

#include "Timer.hpp"

namespace csx::tracking {

ObjectTracker::ObjectTracker(TrackingSettings settings)
    : settings_(std::move(settings)),
      trackManager_(settings_),
      matcher_(settings_.iouThreshold) {}

void ObjectTracker::setSettings(const TrackingSettings& settings) {
    settings_ = settings;
    matcher_ = IoUMatcher(settings_.iouThreshold);
    trackManager_ = TrackManager(settings_);
}

void ObjectTracker::update(const core::Frame& frame, const std::vector<core::Rect2f>& detections,
                           std::vector<core::Track>& outTracks) {
    csx::utils::Timer timer;

    trackManager_.predictAll();

    const auto trackBoxes = trackManager_.trackBoxes();
    std::vector<int> detectionToTrack;
    std::vector<int> unmatchedDetections;
    std::vector<int> unmatchedTracks;
    matcher_.match(detections, trackBoxes, detectionToTrack, unmatchedDetections, unmatchedTracks);

    for (std::size_t detectionIndex = 0; detectionIndex < detections.size(); ++detectionIndex) {
        const auto trackIndex = detectionToTrack[detectionIndex];
        if (trackIndex >= 0) {
            trackManager_.updateMatched(static_cast<std::size_t>(trackIndex),
                                        detections[detectionIndex]);
        }
    }

    for (const auto detectionIndex : unmatchedDetections) {
        trackManager_.createTrack(detections[static_cast<std::size_t>(detectionIndex)]);
    }

    for (const auto trackIndex : unmatchedTracks) {
        trackManager_.markMissed(static_cast<std::size_t>(trackIndex));
    }

    const auto now = frame.valid() ? frame.captureTime : std::chrono::steady_clock::now();
    trackManager_.pruneLostTracks(now);

    outTracks = trackManager_.activeTracks();

    ++processedFrames_;
    lastLatencyMs_ = timer.elapsedMs();
    health_.status = core::EngineStatus::Online;
    health_.detail = "Tracking " + std::to_string(outTracks.size()) + " objects";
    health_.confidence = 1.0F;
    health_.lastLatencyMs = static_cast<float>(lastLatencyMs_);
    health_.processedFrames = processedFrames_;
}

core::EngineHealth ObjectTracker::health() const {
    return health_;
}

std::shared_ptr<core::ITracker> createObjectTracker(TrackingSettings settings) {
    return std::make_shared<ObjectTracker>(std::move(settings));
}

}  // namespace csx::tracking
