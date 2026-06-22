#include <gtest/gtest.h>

#include "SystemMetrics.hpp"

#include <thread>

using csx::utils::SystemMetrics;

TEST(SystemMetricsTest, SamplesMemoryAndCpu) {
    SystemMetrics metrics;
    metrics.sample();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    metrics.sample();

    const auto snapshot = metrics.latest();
    EXPECT_GT(snapshot.ramTotalMb, 0.0F);
    EXPECT_GE(snapshot.ramPercent, 0.0F);
    EXPECT_LE(snapshot.ramPercent, 100.0F);
}
