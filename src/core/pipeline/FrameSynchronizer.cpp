#include "pipeline/FrameSynchronizer.hpp"

namespace csx::core {

void FrameSynchronizer::registerFrame(const Frame& frame) {
    std::lock_guard lock(mutex_);
    latestFrame_ = frame;
}

void FrameSynchronizer::registerTracks(const std::uint64_t frameSequence, std::vector<Track> tracks) {
    std::lock_guard lock(mutex_);
    tracksByFrame_[frameSequence] = std::move(tracks);
    if (tracksByFrame_.size() > 64) {
        tracksByFrame_.erase(tracksByFrame_.begin());
    }
}

void FrameSynchronizer::registerFaces(const std::uint64_t frameSequence, std::vector<FaceMatch> faces) {
    std::lock_guard lock(mutex_);
    facesByFrame_[frameSequence] = std::move(faces);
    if (facesByFrame_.size() > 64) {
        facesByFrame_.erase(facesByFrame_.begin());
    }
}

std::optional<FrameSynchronizer::SyncedSnapshot> FrameSynchronizer::latestSynced() const {
    std::lock_guard lock(mutex_);
    if (!latestFrame_.valid()) {
        return std::nullopt;
    }

    SyncedSnapshot snapshot;
    snapshot.frame = latestFrame_;

    if (const auto trackIt = tracksByFrame_.find(latestFrame_.sequence); trackIt != tracksByFrame_.end()) {
        snapshot.tracks = trackIt->second;
    }
    if (const auto faceIt = facesByFrame_.find(latestFrame_.sequence); faceIt != facesByFrame_.end()) {
        snapshot.faces = faceIt->second;
    }
    return snapshot;
}

}  // namespace csx::core
