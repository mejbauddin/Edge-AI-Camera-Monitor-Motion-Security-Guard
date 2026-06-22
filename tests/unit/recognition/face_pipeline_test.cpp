#include <gtest/gtest.h>

#include "Database.hpp"
#include "FacePipeline.hpp"
#include "FrameConverter.hpp"

#include <filesystem>

using csx::camera::makeSyntheticFrame;
using csx::database::createDatabase;
using csx::recognition::RecognitionSettings;
using csx::recognition::createFacePipeline;

namespace {

void enrollUser(const std::shared_ptr<csx::database::Database>& db,
                const std::shared_ptr<csx::recognition::FacePipeline>& pipeline,
                const std::string& name, const csx::core::Frame& frame) {
    csx::recognition::EnrollmentWizardController wizard(db, pipeline);
    ASSERT_TRUE(wizard.beginEnrollment(name));
    ASSERT_TRUE(wizard.addSample(frame, {10.0F, 10.0F, 40.0F, 40.0F}));
    ASSERT_TRUE(wizard.finalizeEnrollment());
}

}  // namespace

TEST(FacePipelineTest, RecognizesAuthorizedUser) {
    const auto path = (std::filesystem::temp_directory_path() / "csx_recognition_test.sqlite").string();
    std::filesystem::remove(path);

    auto db = createDatabase();
    ASSERT_TRUE(db->open(path));

    RecognitionSettings settings;
    settings.inferenceIntervalFrames = 1;
    settings.matchThreshold = 0.8F;
    auto pipeline = createFacePipeline(db, settings);

    auto enrollFrame = makeSyntheticFrame(128, 96, 1, "primary");
    enrollUser(db, pipeline, "Sentinel Operator", enrollFrame);

    std::vector<csx::core::Track> tracks;
    csx::core::Track track;
    track.id = 1;
    track.active = true;
    track.bbox = {10.0F, 10.0F, 40.0F, 40.0F};
    tracks.push_back(track);

    auto probeFrame = makeSyntheticFrame(128, 96, 2, "primary");
    std::vector<csx::core::FaceMatch> matches;
    pipeline->recognize(probeFrame, tracks, matches);

    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].classification, csx::core::IdentityClassification::Authorized);
    EXPECT_EQ(matches[0].identityName, "Sentinel Operator");

    db->close();
    std::filesystem::remove(path);
}

TEST(FacePipelineTest, MarksUnknownAsFoeWhenAuthorizedExist) {
    const auto path = (std::filesystem::temp_directory_path() / "csx_recognition_foe_test.sqlite").string();
    std::filesystem::remove(path);

    auto db = createDatabase();
    ASSERT_TRUE(db->open(path));

    RecognitionSettings settings;
    settings.inferenceIntervalFrames = 1;
    settings.matchThreshold = 0.35F;
    auto pipeline = createFacePipeline(db, settings);

    enrollUser(db, pipeline, "Known User", makeSyntheticFrame(128, 96, 1, "primary"));

    csx::core::Track track;
    track.id = 2;
    track.active = true;
    track.bbox = {80.0F, 60.0F, 36.0F, 36.0F};

    auto probeFrame = makeSyntheticFrame(128, 96, 2, "primary");
    auto patch = std::make_shared<std::vector<std::uint8_t>>(*probeFrame.bgrData);
    for (std::uint32_t row = 60; row < 96; ++row) {
        for (std::uint32_t col = 80; col < 116; ++col) {
            const auto index = (static_cast<std::size_t>(row) * 128 + col) * 3U;
            (*patch)[index + 0] = 250;
            (*patch)[index + 1] = 250;
            (*patch)[index + 2] = 250;
        }
    }
    probeFrame.bgrData = patch;

    std::vector<csx::core::FaceMatch> matches;
    pipeline->recognize(probeFrame, {track}, matches);

    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].classification, csx::core::IdentityClassification::Foe);

    db->close();
    std::filesystem::remove(path);
}
