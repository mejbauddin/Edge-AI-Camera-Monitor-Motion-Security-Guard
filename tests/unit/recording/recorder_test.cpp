#include <gtest/gtest.h>

#include "Database.hpp"
#include "Mp4Writer.hpp"
#include "Recorder.hpp"

#include <filesystem>

using csx::core::ThreatAssessment;
using csx::core::ThreatLevel;
using csx::database::createDatabase;
using csx::recording::BufferedFrame;
using csx::recording::Mp4Writer;
using csx::recording::Recorder;
using csx::recording::RecorderState;
using csx::recording::RecordingSettings;
using csx::utils::PathResolver;

namespace {

csx::core::Frame makeFrame(std::uint64_t sequence, std::chrono::steady_clock::time_point timestamp) {
    csx::core::Frame frame;
    frame.sequence = sequence;
    frame.cameraId = "cam-1";
    frame.width = 32;
    frame.height = 24;
    frame.captureTime = timestamp;
    frame.bgrData = std::make_shared<const std::vector<std::uint8_t>>(32U * 24U * 3U, 200);
    return frame;
}

ThreatAssessment orangeThreat() {
    ThreatAssessment assessment;
    assessment.level = ThreatLevel::Orange;
    assessment.threatScore = 55.0F;
    return assessment;
}

}  // namespace

TEST(Mp4WriterTest, WritesCsxClipFallback) {
    std::vector<BufferedFrame> frames;
    BufferedFrame buffered;
    buffered.frame = makeFrame(1, std::chrono::steady_clock::now());
    buffered.timestamp = buffered.frame.captureTime;
    frames.push_back(buffered);

    Mp4Writer writer;
    const auto output = std::filesystem::temp_directory_path() / "csx_clip_test";
    const auto result = writer.writeClip(output, frames, 30);

    ASSERT_TRUE(result.success);
    EXPECT_TRUE(std::filesystem::exists(result.outputPath));
    EXPECT_EQ(result.format, "csxclip");
    std::filesystem::remove(result.outputPath);
}

TEST(RecorderTest, SavesThreatTriggeredClip) {
    const auto outputDir = std::filesystem::temp_directory_path() / "csx_recorder_test";
    std::filesystem::remove_all(outputDir);
    std::filesystem::create_directories(outputDir);

    const auto dbPath = outputDir / "test.db";
    auto db = createDatabase();
    ASSERT_TRUE(db->open(dbPath.string()));

    RecordingSettings settings;
    settings.preBufferSeconds = 1;
    settings.postBufferSeconds = 0;
    settings.assumedFps = 10;
    settings.outputDirectory = outputDir.string();
    settings.autoRecordLevel = ThreatLevel::Orange;

    PathResolver resolver;
    resolver.setRootOverride(outputDir.parent_path());

    Recorder recorder(db, settings, resolver);

    const auto start = std::chrono::steady_clock::now();
    for (std::uint64_t i = 0; i < 3; ++i) {
        recorder.feedFrame(makeFrame(i + 1, start + std::chrono::milliseconds(static_cast<int>(i * 100))));
    }

    recorder.triggerRecording(orangeThreat());
    recorder.feedFrame(makeFrame(4, start + std::chrono::milliseconds(350)));
    recorder.stopRecording();

    EXPECT_EQ(recorder.state(), RecorderState::Idle);
    EXPECT_FALSE(db->recordings().listRecordings().empty());

    db->close();
    std::filesystem::remove_all(outputDir);
}
