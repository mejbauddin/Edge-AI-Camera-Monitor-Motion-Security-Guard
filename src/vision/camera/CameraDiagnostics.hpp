#pragma once

#include "types/Frame.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>

namespace csx::camera {

class CameraDiagnostics {
public:
    void onFrameCaptured(std::uint32_t width, std::uint32_t height, float latencyMs, float fps);
    void onFrameDropped();
    void onReconnectAttempt();
    void onConnected(std::uint32_t width, std::uint32_t height);
    void onDisconnected();
    void onError();

    [[nodiscard]] core::CameraDiagnosticsSnapshot snapshot() const;
    [[nodiscard]] std::uint64_t reconnectAttempts() const;

private:
    mutable std::mutex mutex_;
    core::CameraDiagnosticsSnapshot snapshot_;
    std::uint64_t reconnectAttempts_{0};
};

}  // namespace csx::camera
