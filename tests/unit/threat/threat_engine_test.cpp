#include <gtest/gtest.h>

#include "ThreatEngine.hpp"

using csx::core::BehaviorAnomaly;
using csx::core::BehaviorAnomalyType;
using csx::core::FaceMatch;
using csx::core::IdentityClassification;
using csx::core::ThreatLevel;
using csx::core::Track;
using csx::threat::ThreatEngine;
using csx::threat::ThreatSettings;

namespace {

Track makeTrack(std::uint32_t id, float vx = 50.0F) {
    Track track;
    track.id = id;
    track.active = true;
    track.confidence = 0.85F;
    track.velocity = {vx, 10.0F};
    track.bbox = {100.0F, 100.0F, 40.0F, 40.0F};
    return track;
}

FaceMatch makeFoe() {
    FaceMatch match;
    match.trackId = 1;
    match.classification = IdentityClassification::Foe;
    match.confidence = 0.98F;
    return match;
}

BehaviorAnomaly makeIntrusion() {
    BehaviorAnomaly anomaly;
    anomaly.type = BehaviorAnomalyType::Intrusion;
    anomaly.score = 0.95F;
    anomaly.trackId = 1;
    return anomaly;
}

}  // namespace

TEST(ThreatEngineTest, NominalSceneIsGreen) {
    ThreatEngine engine;
    const auto assessment = engine.assess({}, {}, {});
    EXPECT_EQ(assessment.level, ThreatLevel::Green);
    EXPECT_LE(assessment.threatScore, 20.0F);
}

TEST(ThreatEngineTest, FoeWithIntrusionEscalatesThreat) {
    ThreatSettings settings;
    settings.deescalationRate = 1.0F;
    ThreatEngine engine(settings);

    const auto assessment =
        engine.assess({makeTrack(1)}, {makeFoe()}, {makeIntrusion()});

    EXPECT_GE(static_cast<int>(assessment.level), static_cast<int>(ThreatLevel::Orange));
    EXPECT_GT(assessment.threatScore, 50.0F);
    EXPECT_GT(assessment.behaviorScore, 80.0F);
    EXPECT_FALSE(assessment.decisionReason.empty());
    EXPECT_FALSE(assessment.contributingTrackIds.empty());
}

TEST(ThreatEngineTest, MapsDefconFromThreatLevel) {
    ThreatSettings settings;
    settings.deescalationRate = 1.0F;
    ThreatEngine engine(settings);

    const auto assessment =
        engine.assess({makeTrack(1), makeTrack(2, 90.0F)}, {makeFoe()}, {makeIntrusion()});

    EXPECT_GE(static_cast<int>(assessment.defcon), 1);
    EXPECT_LE(static_cast<int>(assessment.defcon), 5);
    EXPECT_TRUE(engine.shouldAutoRecord(assessment.level) ||
                assessment.level == ThreatLevel::Green);
}
