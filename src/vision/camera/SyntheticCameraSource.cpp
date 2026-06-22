#include "SyntheticCameraSource.hpp"

#include "CameraTypes.hpp"
#include "FrameConverter.hpp"

#include "FpsCounter.hpp"
#include "Timer.hpp"

namespace csx::camera {

SyntheticCameraSource::SyntheticCameraSource(CameraSettings settings)
    : settings_(std::move(settings)) {}

bool SyntheticCameraSource::open(const std::string& sourceUri) {
    const auto info = parseSourceUri(sourceUri);
    settings_.cameraId = "synthetic-" + std::to_string(info.deviceIndex);
    open_.store(true);
    sequence_ = 0;
    diagnostics_.state = core::CameraState::Streaming;
    diagnostics_.width = settings_.width;
    diagnostics_.height = settings_.height;
    return true;
}

void SyntheticCameraSource::close() {
    open_.store(false);
    diagnostics_.state = core::CameraState::Disconnected;
}

bool SyntheticCameraSource::grab(core::Frame& outFrame) {
    if (!open_.load()) {
        return false;
    }

    csx::utils::Timer timer;
    static thread_local csx::utils::FpsCounter fpsCounter(30);
    fpsCounter.tick();

    outFrame = makeSyntheticFrame(settings_.width, settings_.height, ++sequence_, settings_.cameraId);
    outFrame.diagnostics = diagnostics_;
    outFrame.diagnostics.fps = static_cast<float>(fpsCounter.fps());
    outFrame.diagnostics.latencyMs = static_cast<float>(timer.elapsedMs());
    diagnostics_ = outFrame.diagnostics;
    return true;
}

bool SyntheticCameraSource::isOpen() const {
    return open_.load();
}

core::CameraDiagnosticsSnapshot SyntheticCameraSource::diagnostics() const {
    return diagnostics_;
}

}  // namespace csx::camera
