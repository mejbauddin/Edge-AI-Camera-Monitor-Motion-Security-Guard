#include <gtest/gtest.h>

#include "pipeline/StageMetrics.hpp"

using csx::core::StageMetrics;

TEST(StageMetricsTest, ComputesRollingAverage) {
    StageMetrics metrics(3);
    metrics.record("motion", std::chrono::microseconds(1000));
    metrics.record("motion", std::chrono::microseconds(3000));
    metrics.record("motion", std::chrono::microseconds(2000));
    metrics.record("motion", std::chrono::microseconds(6000));

    EXPECT_NEAR(metrics.averageMs("motion"), 3.666, 0.01);
    EXPECT_NEAR(metrics.maxMs("motion"), 6.0, 0.01);
    EXPECT_EQ(metrics.sampleCount("motion"), 3U);
}

TEST(StageMetricsTest, UnknownStageReturnsZero) {
    StageMetrics metrics;
    EXPECT_EQ(metrics.averageMs("unknown"), 0.0);
    EXPECT_EQ(metrics.sampleCount("unknown"), 0U);
}
