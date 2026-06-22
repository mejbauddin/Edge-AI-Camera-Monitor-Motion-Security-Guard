#include <gtest/gtest.h>

#include "Timer.hpp"

#include <thread>

using csx::utils::ScopedTimer;
using csx::utils::Timer;

TEST(TimerTest, MeasuresElapsedTime) {
    Timer timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_GE(timer.elapsedMs(), 15.0);
}

TEST(TimerTest, ScopedTimerWritesMilliseconds) {
    double elapsed = 0.0;
  {
    ScopedTimer scoped(elapsed);
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }
    EXPECT_GE(elapsed, 10.0);
}
