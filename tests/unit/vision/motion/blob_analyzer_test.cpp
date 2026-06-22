#include <gtest/gtest.h>

#include "BlobAnalyzer.hpp"

using csx::motion::BlobAnalyzer;

TEST(BlobAnalyzerTest, DetectsConnectedRegion) {
    constexpr std::uint32_t width = 20;
    constexpr std::uint32_t height = 20;
    std::vector<std::uint8_t> mask(width * height, 0);
    for (std::uint32_t y = 5; y < 15; ++y) {
        for (std::uint32_t x = 5; x < 15; ++x) {
            mask[y * width + x] = 255;
        }
    }

    BlobAnalyzer analyzer(50);
    std::vector<csx::core::Rect2f> blobs;
    analyzer.analyze(mask, width, height, 1.0F, 1.0F, blobs);

    ASSERT_EQ(blobs.size(), 1U);
    EXPECT_NEAR(blobs[0].width, 10.0F, 1.0F);
    EXPECT_NEAR(blobs[0].height, 10.0F, 1.0F);
}
