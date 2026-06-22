#include <gtest/gtest.h>

#include "IoUMatcher.hpp"

using csx::tracking::IoUMatcher;
using csx::tracking::computeIoU;

TEST(IoUMatcherTest, ComputesIntersectionOverUnion) {
    csx::core::Rect2f a{0, 0, 10, 10};
    csx::core::Rect2f b{5, 5, 10, 10};
    EXPECT_NEAR(computeIoU(a, b), 25.0F / 175.0F, 0.001F);
}

TEST(IoUMatcherTest, GreedyMatchesHighestIoU) {
    std::vector<csx::core::Rect2f> detections{{0, 0, 10, 10}, {50, 50, 10, 10}};
    std::vector<csx::core::Rect2f> tracks{{1, 1, 10, 10}, {48, 48, 10, 10}};

    IoUMatcher matcher(0.3F);
    std::vector<int> detectionToTrack;
    std::vector<int> unmatchedDetections;
    std::vector<int> unmatchedTracks;
    matcher.match(detections, tracks, detectionToTrack, unmatchedDetections, unmatchedTracks);

    ASSERT_EQ(detectionToTrack.size(), 2U);
    EXPECT_EQ(detectionToTrack[0], 0);
    EXPECT_EQ(detectionToTrack[1], 1);
    EXPECT_TRUE(unmatchedDetections.empty());
    EXPECT_TRUE(unmatchedTracks.empty());
}
