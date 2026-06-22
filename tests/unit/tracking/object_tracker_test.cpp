#include <gtest/gtest.h>

#include "ObjectTracker.hpp"

using csx::tracking::ObjectTracker;
using csx::tracking::TrackingSettings;

namespace {

csx::core::Frame makeFrame(std::uint64_t sequence) {
    csx::core::Frame frame;
    frame.sequence = sequence;
    frame.width = 640;
    frame.height = 480;
    frame.captureTime = std::chrono::steady_clock::now();
    return frame;
}

}  // namespace

TEST(ObjectTrackerTest, MaintainsIdentityAcrossFrames) {
    TrackingSettings settings;
    settings.iouThreshold = 0.2F;
    settings.lostTimeoutMs = 1000;

    ObjectTracker tracker(settings);
    std::vector<csx::core::Track> tracks;

    std::vector<csx::core::Rect2f> detections{{100.0F, 100.0F, 40.0F, 40.0F}};
    tracker.update(makeFrame(1), detections, tracks);
    ASSERT_EQ(tracks.size(), 1U);
    const auto firstId = tracks[0].id;

    detections[0].x += 8.0F;
    tracker.update(makeFrame(2), detections, tracks);
    ASSERT_EQ(tracks.size(), 1U);
    EXPECT_EQ(tracks[0].id, firstId);
    EXPECT_GT(tracks[0].velocity.x, 0.0F);
    EXPECT_GT(tracks[0].history.size(), 1U);
}

TEST(ObjectTrackerTest, CreatesNewTrackForNewDetection) {
    ObjectTracker tracker;
    std::vector<csx::core::Track> tracks;

    tracker.update(makeFrame(1), {{10, 10, 20, 20}}, tracks);
    tracker.update(makeFrame(2), {{10, 10, 20, 20}, {200, 200, 30, 30}}, tracks);
    EXPECT_EQ(tracks.size(), 2U);
}
