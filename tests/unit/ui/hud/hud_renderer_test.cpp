#include "HudRenderer.hpp"

#include "types/Frame.hpp"
#include "types/Enums.hpp"

#include <gtest/gtest.h>

using namespace csx;

// ─── Helpers ─────────────────────────────────────────────────────────────────

/// Build a minimal valid synthetic BGR Frame (3 bytes per pixel, all black).
static core::Frame makeSyntheticFrame(std::uint32_t width = 320,
                                       std::uint32_t height = 240) {
    core::Frame f;
    f.sequence = 1;
    f.cameraId = "test-cam";
    f.width    = width;
    f.height   = height;
    f.bgrData  = std::make_shared<std::vector<std::uint8_t>>(
        static_cast<std::size_t>(width) * height * 3u, std::uint8_t{0});
    return f;
}

static core::Track makeTrack(std::uint32_t id = 1,
                               float x = 50.0F, float y = 50.0F,
                               float w = 60.0F, float h = 80.0F) {
    core::Track t;
    t.id          = id;
    t.active      = true;
    t.bbox        = {x, y, w, h};
    t.center      = {x + w * 0.5F, y + h * 0.5F};
    t.velocity    = {2.0F, 1.0F};
    t.confidence  = 0.85F;
    t.predictedPosition = {x + w * 0.5F + 4.0F, y + h * 0.5F + 2.0F};
    return t;
}

static core::FaceMatch makeFaceMatch(std::uint32_t trackId,
                                      core::IdentityClassification cls,
                                      const std::string& name = "Alice") {
    core::FaceMatch fm;
    fm.trackId        = trackId;
    fm.classification = cls;
    fm.identityName   = name;
    fm.confidence     = 0.92F;
    return fm;
}

static core::ThreatAssessment makeThreat(core::ThreatLevel level = core::ThreatLevel::Green,
                                          float score = 10.0F) {
    core::ThreatAssessment ta;
    ta.level       = level;
    ta.threatScore = score;
    ta.defcon      = core::defconFromThreatLevel(level);
    return ta;
}

// ─── Tests ────────────────────────────────────────────────────────────────────

TEST(HudRendererTest, RendersPassThroughOnValidFrame) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame();
    core::Frame output;

    const std::vector<core::Track> tracks;
    const std::vector<core::FaceMatch> faces;
    const std::vector<core::BehaviorAnomaly> anomalies;
    const core::ThreatAssessment threat = makeThreat();
    const core::SystemHealth health{};

    ASSERT_NO_THROW(
        renderer.render(input, tracks, faces, anomalies, threat, health, output));

    EXPECT_TRUE(output.valid());
    EXPECT_EQ(output.width,  input.width);
    EXPECT_EQ(output.height, input.height);
    EXPECT_EQ(output.bgrData->size(), input.bgrData->size());
}

TEST(HudRendererTest, OutputIsIndependentCopy) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame();
    core::Frame output;

    renderer.render(input, {}, {}, {}, makeThreat(), {}, output);

    // Modifying the output should not affect the input
    ASSERT_TRUE(output.valid());
    (*const_cast<std::vector<std::uint8_t>*>(output.bgrData.get()))[0] = 0xFF;

    EXPECT_EQ((*input.bgrData)[0], 0x00) << "Input frame was mutated";
}

TEST(HudRendererTest, HandlesEmptyTracksGracefully) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame();
    core::Frame output;

    EXPECT_NO_THROW(
        renderer.render(input, {}, {}, {}, makeThreat(), {}, output));
    EXPECT_TRUE(output.valid());
}

TEST(HudRendererTest, RendersWithAuthorizedFaceTrack) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame();
    core::Frame output;

    const auto track = makeTrack(1, 50, 50, 80, 100);
    const auto face  = makeFaceMatch(1, core::IdentityClassification::Authorized, "Alice");

    EXPECT_NO_THROW(
        renderer.render(input, {track}, {face}, {}, makeThreat(core::ThreatLevel::Green, 5.0F), {}, output));
    EXPECT_TRUE(output.valid());
}

TEST(HudRendererTest, RendersWithFoeFaceTrack) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame();
    core::Frame output;

    const auto track = makeTrack(2, 100, 80, 70, 90);
    const auto face  = makeFaceMatch(2, core::IdentityClassification::Foe);

    EXPECT_NO_THROW(
        renderer.render(input, {track}, {face}, {},
                        makeThreat(core::ThreatLevel::Red, 75.0F), {}, output));
    EXPECT_TRUE(output.valid());
}

TEST(HudRendererTest, RendersMultipleTracks) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame(640, 480);
    core::Frame output;

    std::vector<core::Track> tracks;
    for (std::uint32_t i = 0; i < 5; ++i) {
        tracks.push_back(makeTrack(i + 1,
                                   static_cast<float>(i * 100),
                                   static_cast<float>(i * 60),
                                   70.0F, 90.0F));
    }

    EXPECT_NO_THROW(
        renderer.render(input, tracks, {}, {}, makeThreat(core::ThreatLevel::Orange, 55.0F), {}, output));
    EXPECT_TRUE(output.valid());
}

TEST(HudRendererTest, CriticalThreatDoesNotCrash) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame();
    core::Frame output;

    EXPECT_NO_THROW(
        renderer.render(input, {}, {}, {},
                        makeThreat(core::ThreatLevel::Critical, 100.0F), {}, output));
    EXPECT_TRUE(output.valid());
}

TEST(HudRendererTest, RecordingIndicatorToggle) {
    hud::HudRenderer renderer;
    EXPECT_NO_THROW(renderer.setRecording(true));
    EXPECT_NO_THROW(renderer.setRecording(false));

    const core::Frame input = makeSyntheticFrame();
    core::Frame output;
    renderer.setRecording(true);
    renderer.render(input, {}, {}, {}, makeThreat(), {}, output);
    EXPECT_TRUE(output.valid());
}

TEST(HudRendererTest, HealthReturnsOnline) {
    const hud::HudRenderer renderer;
    const auto h = renderer.health();
    EXPECT_EQ(h.status, core::EngineStatus::Online);
}

TEST(HudRendererTest, MultipleSequentialFrames) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame(320, 240);
    const auto threat = makeThreat(core::ThreatLevel::Yellow, 35.0F);
    const auto track  = makeTrack(1, 40, 40, 80, 100);

    for (int i = 0; i < 30; ++i) {
        core::Frame output;
        ASSERT_NO_THROW(
            renderer.render(input, {track}, {}, {}, threat, {}, output));
        ASSERT_TRUE(output.valid());
    }
}

TEST(HudRendererTest, TrackOutsideFrameBoundsHandled) {
    hud::HudRenderer renderer;
    const core::Frame input = makeSyntheticFrame(320, 240);
    core::Frame output;

    // Track bbox almost entirely outside frame
    const auto track = makeTrack(99, 310.0F, 230.0F, 200.0F, 200.0F);

    EXPECT_NO_THROW(
        renderer.render(input, {track}, {}, {}, makeThreat(), {}, output));
    EXPECT_TRUE(output.valid());
}
