#include "CameraEngine.hpp"

#include "CameraSourceFactory.hpp"

#include "events/EventTypes.hpp"

#include <thread>

namespace csx::camera {

CameraEngine::CameraEngine(std::shared_ptr<core::ILogger> logger,
                             std::shared_ptr<core::IEventBus> eventBus)
    : logger_(std::move(logger)),
      eventBus_(std::move(eventBus)),
      frameBuffer_(settings_.bufferDepth) {}

void CameraEngine::configure(const CameraSettings& settings) {
    std::lock_guard lock(mutex_);
    settings_ = settings;
    frameBuffer_.reset(settings_.bufferDepth);
}

bool CameraEngine::start(const std::string& sourceUri) {
    if (running_.exchange(true)) {
        return true;
    }

    activeUri_ = sourceUri;
    reconnectAttempts_ = 0;

    if (!connectSource(sourceUri)) {
        running_.store(false);
        return false;
    }

    if (logger_) {
        logger_->info("CameraEngine", "Camera online: " + sourceUri);
    }

    captureThread_ = std::jthread([this](const std::stop_token stopToken) { captureLoop(stopToken); });
    return true;
}

void CameraEngine::stop() {
    if (!running_.exchange(false)) {
        return;
    }

    captureThread_.request_stop();
    if (captureThread_.joinable()) {
        captureThread_.join();
    }

    if (source_) {
        source_->close();
        source_.reset();
    }

    diagnostics_.onDisconnected();
    if (logger_) {
        logger_->info("CameraEngine", "Camera offline");
    }
}

bool CameraEngine::isRunning() const noexcept {
    return running_.load();
}

bool CameraEngine::popFrame(core::Frame& outFrame, const std::chrono::milliseconds timeout) {
    return frameBuffer_.pop(outFrame, timeout);
}

core::CameraDiagnosticsSnapshot CameraEngine::diagnostics() const {
    return diagnostics_.snapshot();
}

FrameBuffer& CameraEngine::frameBuffer() {
    return frameBuffer_;
}

core::EngineHealth CameraEngine::health() const {
    core::EngineHealth health;
    const auto snapshot = diagnostics_.snapshot();
    health.lastLatencyMs = snapshot.latencyMs;
    health.confidence = snapshot.fps > 0.0F ? 1.0F : 0.0F;

    switch (snapshot.state) {
        case core::CameraState::Streaming:
            health.status = core::EngineStatus::Online;
            health.detail = "Streaming";
            break;
        case core::CameraState::Reconnecting:
            health.status = core::EngineStatus::Degraded;
            health.detail = "Reconnecting";
            break;
        case core::CameraState::Connecting:
            health.status = core::EngineStatus::Starting;
            health.detail = "Connecting";
            break;
        case core::CameraState::Error:
            health.status = core::EngineStatus::Fault;
            health.detail = "Fault";
            break;
        case core::CameraState::Disconnected:
        default:
            health.status = core::EngineStatus::Offline;
            health.detail = "Offline";
            break;
    }
    return health;
}

bool CameraEngine::connectSource(const std::string& sourceUri) {
    source_ = createCameraSource(sourceUri, settings_);
    if (!source_->open(sourceUri)) {
        diagnostics_.onError();
        source_.reset();
        return false;
    }

    const auto snapshot = source_->diagnostics();
    diagnostics_.onConnected(snapshot.width, snapshot.height);
    if (eventBus_) {
        eventBus_->publish(core::events::kCameraState, "STREAMING");
    }
    return true;
}

void CameraEngine::captureLoop(const std::stop_token stopToken) {
    while (!stopToken.stop_requested() && running_.load()) {
        if (!source_ || !source_->isOpen()) {
            diagnostics_.onReconnectAttempt();
            ++reconnectAttempts_;

            if (settings_.maxReconnectAttempts > 0 &&
                reconnectAttempts_ > settings_.maxReconnectAttempts) {
                diagnostics_.onError();
                running_.store(false);
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(settings_.reconnectIntervalMs));
            if (!connectSource(activeUri_)) {
                continue;
            }
        }

        core::Frame frame;
        if (!source_->grab(frame)) {
            diagnostics_.onError();
            if (source_) {
                source_->close();
            }
            continue;
        }

        const auto sourceDiagnostics = source_->diagnostics();
        diagnostics_.onFrameCaptured(sourceDiagnostics.width, sourceDiagnostics.height,
                                     sourceDiagnostics.latencyMs, sourceDiagnostics.fps);

        if (frameBuffer_.size() >= frameBuffer_.capacity()) {
            diagnostics_.onFrameDropped();
        }
        frameBuffer_.push(frame);

        if (eventBus_) {
            eventBus_->publish(core::events::kFrameCaptured, std::to_string(frame.sequence));
        }
    }
}

}  // namespace csx::camera
