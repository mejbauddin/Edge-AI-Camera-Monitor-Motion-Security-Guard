#include <gtest/gtest.h>

#include "ThreatScorer.hpp"

using csx::core::FaceMatch;
using csx::core::IdentityClassification;
using csx::core::Track;
using csx::threat::ThreatScorer;

namespace {

Track activeTrack(std::uint32_t id, float vx = 20.0F) {
    Track track;
    track.id = id;
    track.active = true;
    track.confidence = 0.9F;
    track.velocity = {vx, 0.0F};
    return track;
}

FaceMatch foeFace() {
    FaceMatch match;
    match.trackId = 1;
    match.classification = IdentityClassification::Foe;
    match.confidence = 0.95F;
    return match;
}

}  // namespace

TEST(ThreatScorerTest, FoeFaceProducesHighScore) {
    ThreatScorer scorer;
    EXPECT_GE(scorer.scoreFaces({foeFace()}), 90.0F);
}

TEST(ThreatScorerTest, AuthorizedFaceReducesScore) {
    ThreatScorer scorer;
    FaceMatch authorized;
    authorized.classification = IdentityClassification::Authorized;
    authorized.confidence = 0.99F;
    EXPECT_LE(scorer.scoreFaces({authorized}), 5.0F);
}

TEST(ThreatScorerTest, ComposeWeightsSignals) {
    ThreatScorer scorer;
    const auto scores = scorer.compose({activeTrack(1, 60.0F)}, {foeFace()}, 85.0F, false);
    EXPECT_GT(scores.rawTotal, 40.0F);
    EXPECT_GT(scores.face, 90.0F);
    EXPECT_GT(scores.behavior, 80.0F);
}
