#include <gtest/gtest.h>

#include <algorithm>

#include "BehaviorEngine.hpp"
#include "IntrusionDetector.hpp"
#include "LoiteringDetector.hpp"

using csx::behavior::BehaviorEngine;
using csx::behavior::BehaviorSettings;
using csx::behavior::IntrusionDetector;
using csx::behavior::LoiteringDetector;
using csx::behavior::RestrictedZone;
using csx::core::BehaviorAnomalyType;
using csx::core::Point2f;
using csx::core::Rect2f;
using csx::core::Track;

namespace {

csx::core::Frame makeFrame(std::uint64_t sequence, std::uint32_t width = 640,
                           std::uint32_t height = 480) {
    csx::core::Frame frame;
    frame.sequence = sequence;
    frame.width = width;
    frame.height = height;
    frame.captureTime = std::chrono::steady_clock::now();
    frame.bgrData = std::make_shared<const std::vector<std::uint8_t>>(
        static_cast<std::size_t>(width) * height * 3U, 128);
    return frame;
}

Track makeTrack(std::uint32_t id, float x, float y, float vx = 0.0F, float vy = 0.0F) {
    Track track;
    track.id = id;
    track.active = true;
    track.bbox = {x, y, 40.0F, 40.0F};
    track.center = {x + 20.0F, y + 20.0F};
    track.velocity = {vx, vy};
    return track;
}

BehaviorSettings intrusionSettings() {
    BehaviorSettings settings;
    settings.baselineLearningFrames = 1;
    settings.minAnomalyScore = 0.1F;

    RestrictedZone zone;
    zone.id = "vault";
    zone.restricted = true;
    zone.polygon = {{200.0F, 200.0F}, {400.0F, 200.0F}, {400.0F, 400.0F}, {200.0F, 400.0F}};
    settings.zones = {zone};
    return settings;
}

}  // namespace

TEST(IntrusionDetectorTest, FlagsTrackInsideRestrictedZone) {
    IntrusionDetector detector(intrusionSettings());
    std::vector<csx::core::BehaviorAnomaly> anomalies;

    detector.detect({makeTrack(1, 220.0F, 220.0F)}, anomalies);

    ASSERT_EQ(anomalies.size(), 1U);
    EXPECT_EQ(anomalies[0].type, BehaviorAnomalyType::Intrusion);
    EXPECT_EQ(anomalies[0].trackId, 1U);
}

TEST(IntrusionDetectorTest, IgnoresTrackOutsideRestrictedZone) {
    IntrusionDetector detector(intrusionSettings());
    std::vector<csx::core::BehaviorAnomaly> anomalies;

    detector.detect({makeTrack(2, 20.0F, 20.0F)}, anomalies);
    EXPECT_TRUE(anomalies.empty());
}

TEST(LoiteringDetectorTest, DetectsDwellingTrack) {
    BehaviorSettings settings;
    settings.loiteringThresholdMs = 100;
    settings.loiteringMaxSpeed = 5.0F;
    settings.loiteringRadiusPx = 50.0F;
    settings.minAnomalyScore = 0.1F;

    LoiteringDetector detector(settings);
    std::vector<csx::core::BehaviorAnomaly> anomalies;

    const auto start = std::chrono::steady_clock::now();
    const auto track = makeTrack(7, 100.0F, 100.0F);

    detector.detect({track}, start, anomalies);
    EXPECT_TRUE(anomalies.empty());

    const auto later = start + std::chrono::milliseconds(150);
    detector.detect({track}, later, anomalies);

    ASSERT_EQ(anomalies.size(), 1U);
    EXPECT_EQ(anomalies[0].type, BehaviorAnomalyType::Loitering);
    EXPECT_GE(anomalies[0].duration.count(), 100);
}

TEST(BehaviorEngineTest, LearnsBaselineAndReportsRunning) {
    BehaviorSettings settings;
    settings.baselineLearningFrames = 5;
    settings.runningSpeedThreshold = 30.0F;
    settings.minAnomalyScore = 0.1F;
    settings.zones.clear();

    BehaviorEngine engine(settings);
    std::vector<csx::core::BehaviorAnomaly> anomalies;

    for (std::uint64_t i = 1; i <= 5; ++i) {
        engine.analyze(makeFrame(i), {makeTrack(1, 10.0F, 10.0F, 2.0F, 0.0F)}, anomalies);
    }
    EXPECT_TRUE(engine.baseline().ready());

    engine.analyze(makeFrame(6), {makeTrack(1, 10.0F, 10.0F, 90.0F, 0.0F)}, anomalies);

    const auto running = std::find_if(anomalies.begin(), anomalies.end(),
                                      [](const csx::core::BehaviorAnomaly& anomaly) {
                                          return anomaly.type == BehaviorAnomalyType::Running;
                                      });
    ASSERT_NE(running, anomalies.end());
    EXPECT_GT(running->score, 0.0F);
}

TEST(BehaviorEngineTest, DetectsTamperFromBrightnessShift) {
    BehaviorSettings settings;
    settings.baselineLearningFrames = 3;
    settings.tamperBrightnessDelta = 0.10F;
    settings.minAnomalyScore = 0.1F;
    settings.zones.clear();
    settings.idleCameraMs = 60000;

    BehaviorEngine engine(settings);
    std::vector<csx::core::BehaviorAnomaly> anomalies;

    auto normalFrame = makeFrame(1);
    for (std::uint64_t i = 1; i <= 3; ++i) {
        normalFrame.sequence = i;
        engine.analyze(normalFrame, {}, anomalies);
    }

    auto brightFrame = makeFrame(4);
    brightFrame.bgrData = std::make_shared<const std::vector<std::uint8_t>>(
        static_cast<std::size_t>(brightFrame.width) * brightFrame.height * 3U, 240);
    engine.analyze(brightFrame, {}, anomalies);

    const auto tamper = std::find_if(anomalies.begin(), anomalies.end(),
                                     [](const csx::core::BehaviorAnomaly& anomaly) {
                                         return anomaly.type == BehaviorAnomalyType::CameraTampering;
                                     });
    ASSERT_NE(tamper, anomalies.end());
}
