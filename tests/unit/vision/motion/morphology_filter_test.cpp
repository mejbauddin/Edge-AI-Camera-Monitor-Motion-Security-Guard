#include <gtest/gtest.h>

#include "MorphologyFilter.hpp"

using csx::motion::MorphologyFilter;

TEST(MorphologyFilterTest, OpenRemovesSmallNoise) {
    constexpr std::uint32_t width = 5;
    constexpr std::uint32_t height = 5;
    std::vector<std::uint8_t> mask(width * height, 0);
    mask[12] = 255;

    MorphologyFilter filter(3, 1);
    filter.open(mask, width, height);
    EXPECT_EQ(mask[12], 0);
}

TEST(MorphologyFilterTest, CloseConnectsGaps) {
    constexpr std::uint32_t width = 5;
    constexpr std::uint32_t height = 5;
    std::vector<std::uint8_t> mask(width * height, 0);
    mask[7] = 255;
    mask[9] = 255;

    MorphologyFilter filter(3, 1);
    filter.close(mask, width, height);
    EXPECT_EQ(mask[8], 255);
}
