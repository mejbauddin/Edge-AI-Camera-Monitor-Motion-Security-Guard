#pragma once

namespace csx::utils {

struct SystemMetricsSnapshot {
    float cpuPercent{0.0F};
    float ramPercent{0.0F};
    float ramUsedMb{0.0F};
    float ramTotalMb{0.0F};
    float gpuPercent{0.0F};
    float temperatureC{0.0F};
};

class SystemMetrics {
public:
    SystemMetrics();

    void sample();
    [[nodiscard]] SystemMetricsSnapshot latest() const;

private:
    void sampleCpu();
    void sampleMemory();

    SystemMetricsSnapshot snapshot_{};

#ifdef _WIN32
    bool cpuInitialized_{false};
    unsigned long long prevIdle_{0};
    unsigned long long prevKernel_{0};
    unsigned long long prevUser_{0};
#endif
};

}  // namespace csx::utils
