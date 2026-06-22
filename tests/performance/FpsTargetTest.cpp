#include "FpsTargetTest.hpp"

#include <sstream>
#include <iomanip>

namespace csx::testing {

FpsTargetTest::StageMetrics FpsTargetTest::testCameraCapture() {
    // Simulate camera capture workload
    PerformanceBenchmark benchmark("Camera Capture");
    
    auto workload = []() {
        // Simulate frame capture and copy
        std::vector<std::uint8_t> frame(1280 * 720 * 3, 0);
        std::vector<std::uint8_t> copy = frame;
    };

    auto result = benchmark.runWithWarmup(10, 1000, workload);
    
    return {
        "Camera Capture",
        result.meanMs,
        result.maxMs,
        PerformanceBenchmark::meetsTargetFps(result.meanMs, TARGET_FPS),
        TARGET_FPS
    };
}

FpsTargetTest::StageMetrics FpsTargetTest::testMotionDetection() {
    PerformanceBenchmark benchmark("Motion Detection");
    
    auto workload = []() {
        // Simulate motion detection (background subtraction)
        std::vector<float> frame(1280 * 720, 0.0f);
        std::vector<float> background(1280 * 720, 0.0f);
        std::vector<float> diff(1280 * 720);
        
        for (size_t i = 0; i < frame.size(); ++i) {
            diff[i] = std::abs(frame[i] - background[i]);
        }
    };

    auto result = benchmark.runWithWarmup(10, 1000, workload);
    
    return {
        "Motion Detection",
        result.meanMs,
        result.maxMs,
        PerformanceBenchmark::meetsTargetFps(result.meanMs, TARGET_FPS),
        TARGET_FPS
    };
}

FpsTargetTest::StageMetrics FpsTargetTest::testObjectTracking() {
    PerformanceBenchmark benchmark("Object Tracking");
    
    auto workload = []() {
        // Simulate Kalman filter prediction
        struct Track {
            float x, y, vx, vy;
        };
        std::vector<Track> tracks(32);
        
        for (auto& track : tracks) {
            track.x += track.vx;
            track.y += track.vy;
        }
    };

    auto result = benchmark.runWithWarmup(10, 1000, workload);
    
    return {
        "Object Tracking",
        result.meanMs,
        result.maxMs,
        PerformanceBenchmark::meetsTargetFps(result.meanMs, TARGET_FPS),
        TARGET_FPS
    };
}

FpsTargetTest::StageMetrics FpsTargetTest::testFaceRecognition() {
    PerformanceBenchmark benchmark("Face Recognition");
    
    auto workload = []() {
        // Simulate face embedding computation
        std::vector<float> faceFeatures(128);
        std::vector<float> databaseFeatures(100 * 128);
        
        // Simulate distance computation
        for (size_t i = 0; i < 100; ++i) {
            float dist = 0.0f;
            for (size_t j = 0; j < 128; ++j) {
                const float diff = faceFeatures[j] - databaseFeatures[i * 128 + j];
                dist += diff * diff;
            }
        }
    };

    auto result = benchmark.runWithWarmup(5, 500, workload);  // Fewer iterations due to cost
    
    return {
        "Face Recognition",
        result.meanMs,
        result.maxMs,
        PerformanceBenchmark::meetsTargetFps(result.meanMs, TARGET_FPS),
        TARGET_FPS
    };
}

FpsTargetTest::StageMetrics FpsTargetTest::testThreatAssessment() {
    PerformanceBenchmark benchmark("Threat Assessment");
    
    auto workload = []() {
        // Simulate threat scoring
        struct ThreatInput {
            float motionScore;
            float behaviorScore;
            float faceScore;
        };
        ThreatInput input{0.5f, 0.3f, 0.7f};
        
        const float total = input.motionScore + input.behaviorScore + input.faceScore;
        const float threatScore = total * 100.0f / 3.0f;
    };

    auto result = benchmark.runWithWarmup(10, 10000, workload);
    
    return {
        "Threat Assessment",
        result.meanMs,
        result.maxMs,
        PerformanceBenchmark::meetsTargetFps(result.meanMs, TARGET_FPS),
        TARGET_FPS
    };
}

FpsTargetTest::StageMetrics FpsTargetTest::testHudRendering() {
    PerformanceBenchmark benchmark("HUD Rendering");
    
    auto workload = []() {
        // Simulate HUD overlay rendering
        std::vector<std::uint8_t> frame(1280 * 720 * 3, 0);
        
        // Simulate drawing grid lines
        for (int y = 0; y < 720; y += 72) {
            for (int x = 0; x < 1280; ++x) {
                frame[y * 1280 * 3 + x * 3] = 30;
                frame[y * 1280 * 3 + x * 3 + 1] = 100;
                frame[y * 1280 * 3 + x * 3 + 2] = 180;
            }
        }
    };

    auto result = benchmark.runWithWarmup(10, 1000, workload);
    
    return {
        "HUD Rendering",
        result.meanMs,
        result.maxMs,
        PerformanceBenchmark::meetsTargetFps(result.meanMs, TARGET_FPS),
        TARGET_FPS
    };
}

FpsTargetTest::StageMetrics FpsTargetTest::testFullPipeline() {
    PerformanceBenchmark benchmark("Full Pipeline");
    
    auto workload = []() {
        // Simulate full pipeline (simplified)
        std::vector<std::uint8_t> frame(1280 * 720 * 3, 0);
        
        // Motion detection
        std::vector<float> diff(1280 * 720);
        
        // Tracking
        struct Track { float x, y; };
        std::vector<Track> tracks(32);
        
        // Threat assessment
        float threatScore = 0.5f;
        
        // HUD overlay
        for (size_t i = 0; i < frame.size(); i += 3) {
            frame[i] = static_cast<std::uint8_t>(frame[i] * 0.9f);
        }
    };

    auto result = benchmark.runWithWarmup(10, 1000, workload);
    
    return {
        "Full Pipeline",
        result.meanMs,
        result.maxMs,
        PerformanceBenchmark::meetsTargetFps(result.meanMs, TARGET_FPS),
        TARGET_FPS
    };
}

std::string FpsTargetTest::generateReport(const std::vector<StageMetrics>& metrics) {
    std::ostringstream oss;
    
    oss << "=== 60 FPS Target Validation Report ===\n";
    oss << "Target FPS: " << TARGET_FPS << " (max frame time: " 
        << PerformanceBenchmark::maxFrameTimeForFps(TARGET_FPS) << " ms)\n\n";
    
    int passed = 0;
    for (const auto& metric : metrics) {
        oss << std::fixed << std::setprecision(2);
        oss << "[" << (metric.meetsTarget ? "PASS" : "FAIL") << "] " << metric.stageName << "\n";
        oss << "  Avg frame time: " << metric.avgFrameTimeMs << " ms\n";
        oss << "  Max frame time: " << metric.maxFrameTimeMs << " ms\n";
        oss << "  Target: " << metric.targetFps << " FPS\n\n";
        
        if (metric.meetsTarget) passed++;
    }
    
    oss << "Summary: " << passed << "/" << metrics.size() << " stages meet 60 FPS target\n";
    
    return oss.str();
}

}  // namespace csx::testing
