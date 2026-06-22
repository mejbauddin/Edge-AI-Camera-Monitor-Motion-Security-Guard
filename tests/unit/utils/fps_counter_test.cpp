#include <gtest/gtest.h>

#include "FpsCounter.hpp"

#include <thread>

using csx::utils::FpsCounter;

TEST(FpsCounterTest, ComputesFpsFromTicks) {
    FpsCounter counter(30);
    const auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < 10; ++i) {
        counter.tick(start + std::chrono::milliseconds(i * 16));
    }

    EXPECT_GT(counter.fps(), 50.0);
    EXPECT_GT(counter.frameTimeMs(), 0.0);
    EXPECT_EQ(counter.sampleCount(), 10U);
}

TEST(FpsCounterTest, ResetClearsSamples) {
    FpsCounter counter;
    counter.tick();
    counter.reset();
    EXPECT_EQ(counter.sampleCount(), 0U);
    EXPECT_EQ(counter.fps(), 0.0);
}
