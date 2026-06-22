#include "SystemMetrics.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace csx::utils {

SystemMetrics::SystemMetrics() = default;

void SystemMetrics::sample() {
    sampleCpu();
    sampleMemory();
}

SystemMetricsSnapshot SystemMetrics::latest() const {
    return snapshot_;
}

void SystemMetrics::sampleCpu() {
#ifdef _WIN32
    FILETIME idleTime{};
    FILETIME kernelTime{};
    FILETIME userTime{};
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return;
    }

    const auto toUint64 = [](const FILETIME& value) {
        return (static_cast<unsigned long long>(value.dwHighDateTime) << 32) |
               static_cast<unsigned long long>(value.dwLowDateTime);
    };

    const auto idle = toUint64(idleTime);
    const auto kernel = toUint64(kernelTime);
    const auto user = toUint64(userTime);

    if (!cpuInitialized_) {
        prevIdle_ = idle;
        prevKernel_ = kernel;
        prevUser_ = user;
        cpuInitialized_ = true;
        return;
    }

    const auto idleDelta = idle - prevIdle_;
    const auto kernelDelta = kernel - prevKernel_;
    const auto userDelta = user - prevUser_;
    const auto totalDelta = kernelDelta + userDelta;

    prevIdle_ = idle;
    prevKernel_ = kernel;
    prevUser_ = user;

    if (totalDelta == 0) {
        return;
    }

    const auto used = totalDelta - idleDelta;
    snapshot_.cpuPercent = static_cast<float>((100.0 * static_cast<double>(used)) / static_cast<double>(totalDelta));
#endif
}

void SystemMetrics::sampleMemory() {
#ifdef _WIN32
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) {
        return;
    }

    snapshot_.ramPercent = static_cast<float>(status.dwMemoryLoad);
    snapshot_.ramTotalMb = static_cast<float>(status.ullTotalPhys / (1024ULL * 1024ULL));
    snapshot_.ramUsedMb = static_cast<float>((status.ullTotalPhys - status.ullAvailPhys) / (1024ULL * 1024ULL));
#endif
}

}  // namespace csx::utils
