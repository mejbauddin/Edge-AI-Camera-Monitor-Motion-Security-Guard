#pragma once

#include "CameraDiagnostics.hpp"
#include "CameraTypes.hpp"
#include "FrameBuffer.hpp"
#include "interfaces/Interfaces.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>

namespace csx::camera {

class CameraEngine {
public:
    CameraEngine(std::shared_ptr<core::ILogger> logger, std::shared_ptr<core::IEventBus> eventBus);

    void configure(const CameraSettings& settings);
    bool start(const std::string& sourceUri);
    void stop();

    [[nodiscard]] bool isRunning() const noexcept;
    [[nodiscard]] bool popFrame(core::Frame& outFrame,
                                std::chrono::milliseconds timeout = std::chrono::milliseconds{50});
    [[nodiscard]] core::CameraDiagnosticsSnapshot diagnostics() const;
    [[nodiscard]] FrameBuffer& frameBuffer();
    [[nodiscard]] core::EngineHealth health() const;

private:
    bool connectSource(const std::string& sourceUri);
    void captureLoop(std::stop_token stopToken);

    std::shared_ptr<core::ILogger> logger_;
    std::shared_ptr<core::IEventBus> eventBus_;
    CameraSettings settings_;
  std::unique_ptr<core::ICameraSource> source_;
    FrameBuffer frameBuffer_;
    CameraDiagnostics diagnostics_;
    std::atomic<bool> running_{false};
    std::string activeUri_;
    std::uint32_t reconnectAttempts_{0};
    std::jthread captureThread_;
    mutable std::mutex mutex_;
};

}  // namespace csx::camera
