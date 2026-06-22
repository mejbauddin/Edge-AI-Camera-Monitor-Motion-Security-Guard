#include "PerformanceBenchmark.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace csx::testing {

PerformanceBenchmark::PerformanceBenchmark(std::string name) : name_(std::move(name)) {}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::run(
    std::uint64_t iterations, std::function<void()> workload)
{
    std::vector<double> timings;
    timings.reserve(iterations);

    for (std::uint64_t i = 0; i < iterations; ++i) {
        const auto start = std::chrono::high_resolution_clock::now();
        workload();
        const auto end = std::chrono::high_resolution_clock::now();
        
        const auto duration = std::chrono::duration<double, std::milli>(end - start).count();
        timings.push_back(duration);
    }

    // Calculate statistics
    const double sum = std::accumulate(timings.begin(), timings.end(), 0.0);
    const double mean = sum / static_cast<double>(iterations);
    
    const auto [minIt, maxIt] = std::minmax_element(timings.begin(), timings.end());
    const double minVal = *minIt;
    const double maxVal = *maxIt;
    
    // Standard deviation
    double variance = 0.0;
    for (double t : timings) {
        variance += (t - mean) * (t - mean);
    }
    variance /= static_cast<double>(iterations);
    const double stdDev = std::sqrt(variance);

    // Throughput (operations per second)
    const double totalTimeSec = sum / 1000.0;
    const double throughput = static_cast<double>(iterations) / totalTimeSec;

    return {
        name_,
        mean,
        minVal,
        maxVal,
        stdDev,
        iterations,
        throughput
    };
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::runWithWarmup(
    std::uint64_t warmupIterations,
    std::uint64_t benchmarkIterations,
    std::function<void()> workload)
{
    // Warmup phase
    for (std::uint64_t i = 0; i < warmupIterations; ++i) {
        workload();
    }

    return run(benchmarkIterations, workload);
}

bool PerformanceBenchmark::meetsTargetFps(double frameTimeMs, double targetFps) {
    const double maxTime = maxFrameTimeForFps(targetFps);
    return frameTimeMs <= maxTime;
}

double PerformanceBenchmark::maxFrameTimeForFps(double targetFps) {
    return 1000.0 / targetFps;
}

// ══════════════════════════════════════════════════════════════════════════════
// StressTest
// ══════════════════════════════════════════════════════════════════════════════

StressTest::StressResult StressTest::memoryStressTest(std::uint64_t allocationSize,
                                                       std::uint64_t iterations)
{
    StressResult result;
    result.passed = true;
    result.totalOperations = iterations;
    result.memoryLeakedBytes = 0;

    try {
        std::vector<std::vector<std::uint8_t>> allocations;
        allocations.reserve(iterations);

        const auto start = std::chrono::high_resolution_clock::now();

        for (std::uint64_t i = 0; i < iterations; ++i) {
            allocations.emplace_back(allocationSize, 0xFF);
        }

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        result.avgLatencyMs = duration / static_cast<double>(iterations);
        result.maxLatencyMs = duration;  // Approximation

        // Clean up
        allocations.clear();

    } catch (const std::bad_alloc&) {
        result.passed = false;
        result.failureReason = "Memory allocation failed";
    }

    return result;
}

StressTest::StressResult StressTest::cpuStressTest(std::function<void()> workload,
                                                    std::uint64_t durationMs)
{
    StressResult result;
    result.passed = true;
    result.totalOperations = 0;

    const auto startTime = std::chrono::steady_clock::now();
    const auto endTime = startTime + std::chrono::milliseconds(durationMs);

    std::vector<double> latencies;

    while (std::chrono::steady_clock::now() < endTime) {
        const auto start = std::chrono::high_resolution_clock::now();
        workload();
        const auto end = std::chrono::high_resolution_clock::now();
        
        const auto duration = std::chrono::duration<double, std::milli>(end - start).count();
        latencies.push_back(duration);
        result.totalOperations++;
    }

    if (!latencies.empty()) {
        const double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
        result.avgLatencyMs = sum / static_cast<double>(latencies.size());
        const auto [minIt, maxIt] = std::minmax_element(latencies.begin(), latencies.end());
        result.maxLatencyMs = *maxIt;
    }

    return result;
}

StressTest::StressResult StressTest::threadStressTest(std::uint64_t threadCount,
                                                         std::function<void()> workload)
{
    StressResult result;
    result.passed = true;
    result.totalOperations = 0;

    std::vector<std::thread> threads;
    std::vector<std::uint64_t> operationCounts(threadCount, 0);
    std::vector<double> maxLatencies(threadCount, 0.0);

    const auto start = std::chrono::steady_clock::now();

    for (std::uint64_t i = 0; i < threadCount; ++i) {
        threads.emplace_back([&, i]() {
            const auto threadStart = std::chrono::steady_clock::now();
            std::uint64_t ops = 0;
            double maxLat = 0.0;

            while (std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::steady_clock::now() - threadStart).count() < 5) {
                const auto opStart = std::chrono::high_resolution_clock::now();
                workload();
                const auto opEnd = std::chrono::high_resolution_clock::now();
                
                const auto latency = std::chrono::duration<double, std::milli>(opEnd - opStart).count();
                maxLat = std::max(maxLat, latency);
                ops++;
            }

            operationCounts[i] = ops;
            maxLatencies[i] = maxLat;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    result.totalOperations = std::accumulate(operationCounts.begin(), operationCounts.end(), 0ULL);
    result.avgLatencyMs = 0.0;  // Would need more detailed tracking
    result.maxLatencyMs = *std::max_element(maxLatencies.begin(), maxLatencies.end());

    return result;
}

}  // namespace csx::testing
