#include <gtest/gtest.h>

#include "FrameConverter.hpp"
#include "MotionEngine.hpp"

#include "CvHelpers.hpp"

#include <vector>

using csx::camera::makeBgrFrame;
using csx::motion::MotionEngine;
using csx::motion::MotionSettings;

namespace {

csx::core::Frame makeSolidFrame(std::uint32_t width, std::uint32_t height, std::uint8_t blue,
                                std::uint8_t green, std::uint8_t red, std::uint64_t sequence) {
    std::vector<std::uint8_t> data(csx::utils::bgrBufferSize(width, height));
    for (std::size_t i = 0; i < data.size(); i += 3U) {
        data[i + 0] = blue;
        data[i + 1] = green;
        data[i + 2] = red;
    }
    return makeBgrFrame(data.data(), width, height, sequence, "test");
}

}  // namespace

TEST(MotionEngineTest, DetectsMovingObject) {
    MotionSettings settings;
    settings.processingWidth = 64;
    settings.processingHeight = 48;
    settings.minBlobArea = 20;
    settings.threshold = 10;
    settings.learningRate = 0.2;
    settings.shadowRemoval = false;

    MotionEngine engine(settings);
    std::vector<csx::core::Rect2f> blobs;

    const auto background = makeSolidFrame(64, 48, 20, 20, 20, 1);
    engine.process(background, blobs);
    EXPECT_TRUE(blobs.empty());

    auto moving = makeSolidFrame(64, 48, 20, 20, 20, 2);
    auto movingData = std::make_shared<std::vector<std::uint8_t>>(*moving.bgrData);
    for (std::uint32_t y = 10; y < 20; ++y) {
        for (std::uint32_t x = 10; x < 20; ++x) {
            const auto index = (static_cast<std::size_t>(y) * 64 + x) * 3U;
            (*movingData)[index + 0] = 240;
            (*movingData)[index + 1] = 240;
            (*movingData)[index + 2] = 240;
        }
    }
    moving.bgrData = movingData;

    engine.process(moving, blobs);
    ASSERT_FALSE(blobs.empty());
    EXPECT_GT(blobs[0].width, 5.0F);
    EXPECT_GT(blobs[0].height, 5.0F);
    EXPECT_EQ(engine.health().status, csx::core::EngineStatus::Online);
}
