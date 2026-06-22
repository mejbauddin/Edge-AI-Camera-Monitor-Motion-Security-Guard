#include "TrackManager.hpp"

#include <algorithm>

namespace csx::tracking {

TrackManager::TrackManager(TrackingSettings settings) : settings_(std::move(settings)) {}

void TrackManager::predictAll() {
    for (auto& managed : tracks_) {
        managed.kalman.predict();
        managed.track.predictedPosition = managed.kalman.predictedPosition();
    }
}

void TrackManager::pushHistory(ManagedTrack& managedTrack, const core::Point2f& point) {
    managedTrack.track.history.push_back(point);
    if (managedTrack.track.history.size() > settings_.historyLength) {
        managedTrack.track.history.erase(managedTrack.track.history.begin());
    }
}

float TrackManager::computeConfidence(const ManagedTrack& managedTrack) const noexcept {
    const auto missPenalty = static_cast<float>(managedTrack.missedFrames) * 0.15F;
    const auto ageBonus = std::min(static_cast<float>(managedTrack.track.ageFrames) * 0.02F, 0.4F);
    return std::max(0.0F, std::min(1.0F, 0.5F + ageBonus - missPenalty));
}

void TrackManager::updateMatched(const std::size_t trackIndex, const core::Rect2f& detection) {
    if (trackIndex >= tracks_.size()) {
        return;
    }

    auto& managed = tracks_[trackIndex];
    managed.missedFrames = 0;
    managed.lastSeen = std::chrono::steady_clock::now();
    managed.track.bbox = detection;
    managed.track.center = detection.center();
    managed.kalman.update(managed.track.center);
    managed.velocity.update(managed.track.center, managed.track.velocity, managed.track.acceleration,
                            managed.track.directionDegrees);
    managed.track.predictedPosition = managed.kalman.predictedPosition();
    managed.track.ageFrames += 1;
    managed.track.active = true;
    managed.track.confidence = computeConfidence(managed);
    pushHistory(managed, managed.track.center);
}

std::uint32_t TrackManager::createTrack(const core::Rect2f& detection) {
    if (tracks_.size() >= settings_.maxTracks) {
        return 0;
    }

    ManagedTrack managed;
    managed.track.id = nextTrackId_++;
    managed.track.bbox = detection;
    managed.track.center = detection.center();
    managed.track.active = true;
    managed.track.ageFrames = 1;
    managed.track.confidence = 0.5F;
    managed.kalman.reset(managed.track.center);
    managed.velocity.reset(managed.track.center);
    managed.track.predictedPosition = managed.kalman.predictedPosition();
    managed.lastSeen = std::chrono::steady_clock::now();
    pushHistory(managed, managed.track.center);
    tracks_.push_back(std::move(managed));
    return tracks_.back().track.id;
}

void TrackManager::markMissed(const std::size_t trackIndex) {
    if (trackIndex >= tracks_.size()) {
        return;
    }
    ++tracks_[trackIndex].missedFrames;
    tracks_[trackIndex].track.confidence = computeConfidence(tracks_[trackIndex]);
}

void TrackManager::pruneLostTracks(const std::chrono::steady_clock::time_point now) {
    const auto timeout = std::chrono::milliseconds(settings_.lostTimeoutMs);
    tracks_.erase(std::remove_if(tracks_.begin(), tracks_.end(),
                                 [&](const ManagedTrack& managed) {
                                     if (managed.missedFrames == 0) {
                                         return false;
                                     }
                                     return now - managed.lastSeen > timeout;
                                 }),
                  tracks_.end());
}

std::vector<core::Rect2f> TrackManager::trackBoxes() const {
    std::vector<core::Rect2f> boxes;
    boxes.reserve(tracks_.size());
    for (const auto& managed : tracks_) {
        boxes.push_back(managed.track.bbox);
    }
    return boxes;
}

std::vector<core::Track> TrackManager::activeTracks() const {
    std::vector<core::Track> active;
    active.reserve(tracks_.size());
    for (const auto& managed : tracks_) {
        if (managed.track.active) {
            active.push_back(managed.track);
        }
    }
    return active;
}

std::size_t TrackManager::size() const noexcept {
    return tracks_.size();
}

}  // namespace csx::tracking
