#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>
#include <cstdint>

namespace csx::testing {

// ──────────────────────────────────────────────────────────────────────────────
// PerformanceBenchmark — measures execution time and throughput
// ──────────────────────────────────────────────────────────────────────────────
class PerformanceBenchmark {
public:
    struct BenchmarkResult {
        std::string name;
        double meanMs;
        double minMs;
        double maxMs;
        double stdDevMs;
        std::uint64_t iterations;
        double throughput;  // operations per second
    };

    explicit PerformanceBenchmark(std::string name);
    ~PerformanceBenchmark() = default;

    // Run benchmark with multiple iterations
    BenchmarkResult run(std::uint64_t iterations, std::function<void()> workload);

    // Run benchmark with warmup
    BenchmarkResult runWithWarmup(std::uint64_t warmupIterations,
                                   std::uint64_t benchmarkIterations,
                                   std::function<void()> workload);

    // Target FPS validation (for 60 FPS target)
    static bool meetsTargetFps(double frameTimeMs, double targetFps = 60.0);
    static double maxFrameTimeForFps(double targetFps = 60.0);

private:
    std::string name_;
};

// ──────────────────────────────────────────────────────────────────────────────
// StressTest — simulates high-load conditions
// ──────────────────────────────────────────────────────────────────────────────
class StressTest {
public:
    struct StressResult {
        bool passed;
        std::string failureReason;
        std::uint64_t totalOperations;
        double avgLatencyMs;
        double maxLatencyMs;
        std::uint64_t memoryLeakedBytes;
    };

    StressTest() = default;
    ~StressTest() = default;

    // Memory stress test
    StressTest memoryStressTest(std::uint64_t allocationSize, std::uint64_t iterations);

    // CPU stress test
    StressTest cpuStressTest(std::function<void()> workload, std::uint64_t durationMs);

    // Concurrent thread stress test
    StressTest threadStressTest(std::uint64_t threadCount, std::function<void()> workload);
};

}  // namespace csx::testing
