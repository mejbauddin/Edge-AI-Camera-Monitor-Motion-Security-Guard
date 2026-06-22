#include <gtest/gtest.h>

#include "threading/ThreadPool.hpp"

#include <atomic>
#include <future>
#include <vector>

using csx::core::ThreadPool;

TEST(ThreadPoolTest, ExecutesSubmittedTasks) {
    ThreadPool pool(2);
    std::atomic<int> counter{0};

    auto futureA = pool.submit([&counter] { counter.fetch_add(1); });
    auto futureB = pool.submit([&counter] { counter.fetch_add(10); });
    futureA.get();
    futureB.get();

    EXPECT_EQ(counter.load(), 11);
    pool.shutdown();
}

TEST(ThreadPoolTest, ParallelThroughput) {
    ThreadPool pool(4);
    std::atomic<int> counter{0};

    std::vector<std::future<void>> futures;
    futures.reserve(20);
    for (int i = 0; i < 20; ++i) {
        futures.push_back(pool.submit([&counter] { counter.fetch_add(1); }));
    }
    for (auto& future : futures) {
        future.get();
    }
    EXPECT_EQ(counter.load(), 20);
    pool.shutdown();
}
