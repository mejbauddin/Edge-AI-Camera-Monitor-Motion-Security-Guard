#pragma once

#include "PerformanceBenchmark.hpp"
#include <string>

namespace csx::testing {

// ──────────────────────────────────────────────────────────────────────────────
// FpsTargetTest — validates 60 FPS target for all pipeline stages
// ──────────────────────────────────────────────────────────────────────────────
class FpsTargetTest {
public:
    struct StageMetrics {
        std::string stageName;
        double avgFrameTimeMs;
        double maxFrameTimeMs;
        bool meetsTarget;
        double targetFps;
    };

    FpsTargetTest() = default;
    ~FpsTargetTest() = default;

    // Test individual pipeline stages
    StageMetrics testCameraCapture();
    StageMetrics testMotionDetection();
    StageMetrics testObjectTracking();
    StageMetrics testFaceRecognition();
    StageMetrics testThreatAssessment();
    StageMetrics testHudRendering();
    StageMetrics testFullPipeline();

    // Generate report
    std::string generateReport(const std::vector<StageMetrics>& metrics);

private:
    static constexpr double TARGET_FPS = 60.0;
};

}  // namespace csx::testing
