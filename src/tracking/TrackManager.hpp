#pragma once

#include "KalmanPredictor.hpp"
#include "TrackingSettings.hpp"
#include "VelocityEstimator.hpp"
#include "types/Frame.hpp"

#include <chrono>
#include <cstdint>
#include <vector>

namespace csx::tracking {

struct ManagedTrack {
    core::Track track;
    KalmanPredictor kalman;
    VelocityEstimator velocity;
    std::chrono::steady_clock::time_point lastSeen{};
    std::uint32_t missedFrames{0};
};

class TrackManager {
public:
    explicit TrackManager(TrackingSettings settings = {});

    void predictAll();
    void updateMatched(std::size_t trackIndex, const core::Rect2f& detection);
    std::uint32_t createTrack(const core::Rect2f& detection);
    void markMissed(std::size_t trackIndex);
    void pruneLostTracks(std::chrono::steady_clock::time_point now);

    [[nodiscard]] std::vector<core::Rect2f> trackBoxes() const;
    [[nodiscard]] std::vector<core::Track> activeTracks() const;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    void pushHistory(ManagedTrack& managedTrack, const core::Point2f& point);
    [[nodiscard]] float computeConfidence(const ManagedTrack& managedTrack) const noexcept;

    TrackingSettings settings_;
    std::vector<ManagedTrack> tracks_;
    std::uint32_t nextTrackId_{1};
};

}  // namespace csx::tracking
