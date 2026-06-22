#include <gtest/gtest.h>

#include "CvHelpers.hpp"

#include <cstdint>
#include <vector>

using csx::utils::bgrBufferSize;
using csx::utils::clampValue;
using csx::utils::copyBgrRegion;
using csx::utils::fillBgr;
using csx::utils::resizeBgrNearestNeighbor;

TEST(CvHelpersTest, ResizeDownscalesBgrImage) {
    constexpr std::uint32_t width = 4;
    constexpr std::uint32_t height = 4;
    std::vector<std::uint8_t> source(bgrBufferSize(width, height), 0);
    source[0] = 255;
    source[1] = 0;
    source[2] = 0;

    std::vector<std::uint8_t> destination(bgrBufferSize(2, 2), 0);
    resizeBgrNearestNeighbor(source.data(), width, height, destination.data(), 2, 2);

    EXPECT_EQ(destination[0], 255);
    EXPECT_EQ(destination[1], 0);
    EXPECT_EQ(destination[2], 0);
}

TEST(CvHelpersTest, CopyRegionExtractsPixels) {
    constexpr std::uint32_t width = 4;
    constexpr std::uint32_t height = 4;
    std::vector<std::uint8_t> source(bgrBufferSize(width, height), 0);
    source[bgrBufferSize(width, height) - 3] = 10;
    source[bgrBufferSize(width, height) - 2] = 20;
    source[bgrBufferSize(width, height) - 1] = 30;

    std::vector<std::uint8_t> region;
    copyBgrRegion(source.data(), width, height, 3, 3, 1, 1, region);

    ASSERT_EQ(region.size(), 3U);
    EXPECT_EQ(region[0], 10);
    EXPECT_EQ(region[1], 20);
    EXPECT_EQ(region[2], 30);
}

TEST(CvHelpersTest, ClampValueLimitsRange) {
    EXPECT_EQ(clampValue(15, 0, 10), 10);
    EXPECT_EQ(clampValue(-5, 0, 10), 0);
    EXPECT_EQ(clampValue(7, 0, 10), 7);
}

TEST(CvHelpersTest, FillBgrSetsAllPixels) {
    std::vector<std::uint8_t> buffer(bgrBufferSize(2, 2), 0);
    fillBgr(buffer.data(), 2, 2, 1, 2, 3);
    EXPECT_EQ(buffer[0], 1);
    EXPECT_EQ(buffer[1], 2);
    EXPECT_EQ(buffer[2], 3);
    EXPECT_EQ(buffer[buffer.size() - 1], 3);
}
