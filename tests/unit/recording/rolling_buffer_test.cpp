#include <gtest/gtest.h>

#include "RollingBuffer.hpp"

using csx::recording::BufferedFrame;
using csx::recording::RecordingSettings;
using csx::recording::RollingBuffer;

namespace {

csx::core::Frame makeFrame(std::uint64_t sequence, std::chrono::steady_clock::time_point timestamp) {
    csx::core::Frame frame;
    frame.sequence = sequence;
    frame.width = 64;
    frame.height = 48;
    frame.captureTime = timestamp;
    frame.bgrData = std::make_shared<const std::vector<std::uint8_t>>(64U * 48U * 3U, 128);
    return frame;
}

}  // namespace

TEST(RollingBufferTest, RetainsRecentFramesWithinWindow) {
    RecordingSettings settings;
    settings.preBufferSeconds = 2;
    settings.assumedFps = 10;

    RollingBuffer buffer(settings);
    const auto start = std::chrono::steady_clock::now();

    for (std::uint64_t i = 0; i < 5; ++i) {
        BufferedFrame buffered;
        buffered.frame = makeFrame(i + 1, start + std::chrono::milliseconds(static_cast<int>(i * 200)));
        buffered.timestamp = buffered.frame.captureTime;
        buffer.push(std::move(buffered));
    }

    EXPECT_GE(buffer.size(), 3U);
    const auto snapshot = buffer.snapshot();
    EXPECT_FALSE(snapshot.empty());
    EXPECT_EQ(snapshot.front().frame.sequence, 1U);
}

TEST(RollingBufferTest, DropsExpiredFrames) {
    RecordingSettings settings;
    settings.preBufferSeconds = 1;
    settings.assumedFps = 5;

    RollingBuffer buffer(settings);
    const auto start = std::chrono::steady_clock::now();

    BufferedFrame old;
    old.frame = makeFrame(1, start - std::chrono::seconds(5));
    old.timestamp = old.frame.captureTime;
    buffer.push(std::move(old));

    BufferedFrame recent;
    recent.frame = makeFrame(2, start);
    recent.timestamp = recent.frame.captureTime;
    buffer.push(std::move(recent));

    ASSERT_EQ(buffer.size(), 1U);
    EXPECT_EQ(buffer.snapshot().front().frame.sequence, 2U);
}
