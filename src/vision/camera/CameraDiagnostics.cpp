#include "CameraDiagnostics.hpp"

namespace csx::camera {

void CameraDiagnostics::onFrameCaptured(const std::uint32_t width, const std::uint32_t height,
                                        const float latencyMs, const float fps) {
    std::lock_guard lock(mutex_);
    snapshot_.width = width;
    snapshot_.height = height;
    snapshot_.latencyMs = latencyMs;
    snapshot_.fps = fps;
    snapshot_.state = core::CameraState::Streaming;
}

void CameraDiagnostics::onFrameDropped() {
    std::lock_guard lock(mutex_);
    ++snapshot_.droppedFrames;
}

void CameraDiagnostics::onReconnectAttempt() {
    std::lock_guard lock(mutex_);
    ++reconnectAttempts_;
    snapshot_.state = core::CameraState::Reconnecting;
}

void CameraDiagnostics::onConnected(const std::uint32_t width, const std::uint32_t height) {
    std::lock_guard lock(mutex_);
    snapshot_.width = width;
    snapshot_.height = height;
    snapshot_.state = core::CameraState::Streaming;
}

void CameraDiagnostics::onDisconnected() {
    std::lock_guard lock(mutex_);
    snapshot_.state = core::CameraState::Disconnected;
}

void CameraDiagnostics::onError() {
    std::lock_guard lock(mutex_);
    snapshot_.state = core::CameraState::Error;
}

core::CameraDiagnosticsSnapshot CameraDiagnostics::snapshot() const {
    std::lock_guard lock(mutex_);
    return snapshot_;
}

std::uint64_t CameraDiagnostics::reconnectAttempts() const {
    std::lock_guard lock(mutex_);
    return reconnectAttempts_;
}

}  // namespace csx::camera
