#include "OpenCvCameraSources.hpp"

#include "CameraTypes.hpp"
#include "FrameConverter.hpp"

#include "CvHelpers.hpp"
#include "FpsCounter.hpp"
#include "Timer.hpp"

#if defined(CSX_HAS_OPENCV)
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#endif

namespace csx::camera {

#if defined(CSX_HAS_OPENCV)

struct OpenCvCaptureBackend::Impl {
    cv::VideoCapture capture;
    csx::utils::FpsCounter fpsCounter{30};
};

OpenCvCaptureBackend::OpenCvCaptureBackend(CameraSettings settings)
    : impl_(std::make_unique<Impl>()), settings_(settings) {}

OpenCvCaptureBackend::~OpenCvCaptureBackend() = default;

bool OpenCvCaptureBackend::openUsb(const int deviceIndex) {
    close();
    std::lock_guard lock(mutex_);
    if (!impl_->capture.open(deviceIndex, cv::CAP_DSHOW)) {
        diagnostics_.state = core::CameraState::Error;
        return false;
    }
    impl_->capture.set(cv::CAP_PROP_FRAME_WIDTH, static_cast<double>(settings_.width));
    impl_->capture.set(cv::CAP_PROP_FRAME_HEIGHT, static_cast<double>(settings_.height));
    impl_->capture.set(cv::CAP_PROP_FPS, static_cast<double>(settings_.targetFps));
    diagnostics_.width = static_cast<std::uint32_t>(impl_->capture.get(cv::CAP_PROP_FRAME_WIDTH));
    diagnostics_.height = static_cast<std::uint32_t>(impl_->capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    diagnostics_.state = core::CameraState::Streaming;
    return true;
}

bool OpenCvCaptureBackend::openStream(const std::string& url, const bool preferFfmpeg) {
    close();
    std::lock_guard lock(mutex_);
    const auto api = preferFfmpeg ? cv::CAP_FFMPEG : cv::CAP_ANY;
    if (!impl_->capture.open(url, api)) {
        diagnostics_.state = core::CameraState::Error;
        return false;
    }
    diagnostics_.width = static_cast<std::uint32_t>(impl_->capture.get(cv::CAP_PROP_FRAME_WIDTH));
    diagnostics_.height = static_cast<std::uint32_t>(impl_->capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    if (diagnostics_.width == 0) {
        diagnostics_.width = settings_.width;
    }
    if (diagnostics_.height == 0) {
        diagnostics_.height = settings_.height;
    }
    diagnostics_.state = core::CameraState::Streaming;
    return true;
}

void OpenCvCaptureBackend::close() {
    std::lock_guard lock(mutex_);
    if (impl_->capture.isOpened()) {
        impl_->capture.release();
    }
    diagnostics_.state = core::CameraState::Disconnected;
}

bool OpenCvCaptureBackend::isOpen() const {
    std::lock_guard lock(mutex_);
    return impl_->capture.isOpened();
}

bool OpenCvCaptureBackend::read(core::Frame& outFrame) {
    std::lock_guard lock(mutex_);
    if (!impl_->capture.isOpened()) {
        return false;
    }

    csx::utils::Timer timer;
    cv::Mat bgr;
    {
        std::lock_guard<std::mutex> cvLock(csx::utils::openCvMutex());
        if (!impl_->capture.read(bgr) || bgr.empty()) {
            diagnostics_.state = core::CameraState::Error;
            return false;
        }

        if (bgr.channels() == 4) {
            cv::cvtColor(bgr, bgr, cv::COLOR_BGRA2BGR);
        } else if (bgr.channels() == 1) {
            cv::cvtColor(bgr, bgr, cv::COLOR_GRAY2BGR);
        }
    }

    impl_->fpsCounter.tick();
    outFrame = makeBgrFrame(bgr.data, static_cast<std::uint32_t>(bgr.cols),
                            static_cast<std::uint32_t>(bgr.rows), ++sequence_, settings_.cameraId);
    diagnostics_.width = outFrame.width;
    diagnostics_.height = outFrame.height;
    diagnostics_.fps = static_cast<float>(impl_->fpsCounter.fps());
    diagnostics_.latencyMs = static_cast<float>(timer.elapsedMs());
    diagnostics_.state = core::CameraState::Streaming;
    outFrame.diagnostics = diagnostics_;
    return true;
}

core::CameraDiagnosticsSnapshot OpenCvCaptureBackend::diagnostics() const {
    std::lock_guard lock(mutex_);
    return diagnostics_;
}

UsbCameraSource::UsbCameraSource(CameraSettings settings)
    : settings_(settings), backend_(settings_) {}

bool UsbCameraSource::open(const std::string& sourceUri) {
    const auto info = parseSourceUri(sourceUri);
    settings_.cameraId = "usb-" + std::to_string(info.deviceIndex);
    return backend_.openUsb(info.deviceIndex);
}

void UsbCameraSource::close() { backend_.close(); }
bool UsbCameraSource::grab(core::Frame& outFrame) { return backend_.read(outFrame); }
bool UsbCameraSource::isOpen() const { return backend_.isOpen(); }
core::CameraDiagnosticsSnapshot UsbCameraSource::diagnostics() const { return backend_.diagnostics(); }

RtspCameraSource::RtspCameraSource(CameraSettings settings)
    : settings_(settings), backend_(settings_) {}

bool RtspCameraSource::open(const std::string& sourceUri) {
    settings_.cameraId = "rtsp";
    return backend_.openStream(sourceUri, true);
}

void RtspCameraSource::close() { backend_.close(); }
bool RtspCameraSource::grab(core::Frame& outFrame) { return backend_.read(outFrame); }
bool RtspCameraSource::isOpen() const { return backend_.isOpen(); }
core::CameraDiagnosticsSnapshot RtspCameraSource::diagnostics() const { return backend_.diagnostics(); }

IpCameraSource::IpCameraSource(CameraSettings settings)
    : settings_(settings), backend_(settings_) {}

bool IpCameraSource::open(const std::string& sourceUri) {
    settings_.cameraId = "ip";
    const auto info = parseSourceUri(sourceUri);
    return backend_.openStream(info.uri, false);
}

void IpCameraSource::close() { backend_.close(); }
bool IpCameraSource::grab(core::Frame& outFrame) { return backend_.read(outFrame); }
bool IpCameraSource::isOpen() const { return backend_.isOpen(); }
core::CameraDiagnosticsSnapshot IpCameraSource::diagnostics() const { return backend_.diagnostics(); }

#else

struct OpenCvCaptureBackend::Impl {};

OpenCvCaptureBackend::OpenCvCaptureBackend(CameraSettings settings) : settings_(settings) {}
OpenCvCaptureBackend::~OpenCvCaptureBackend() = default;
bool OpenCvCaptureBackend::openUsb(int) { return false; }
bool OpenCvCaptureBackend::openStream(const std::string&, bool) { return false; }
void OpenCvCaptureBackend::close() { diagnostics_.state = core::CameraState::Disconnected; }
bool OpenCvCaptureBackend::isOpen() const { return false; }
bool OpenCvCaptureBackend::read(core::Frame&) { return false; }
core::CameraDiagnosticsSnapshot OpenCvCaptureBackend::diagnostics() const { return diagnostics_; }

UsbCameraSource::UsbCameraSource(CameraSettings settings)
    : settings_(settings), backend_(settings_) {}
bool UsbCameraSource::open(const std::string&) { return false; }
void UsbCameraSource::close() {}
bool UsbCameraSource::grab(core::Frame&) { return false; }
bool UsbCameraSource::isOpen() const { return false; }
core::CameraDiagnosticsSnapshot UsbCameraSource::diagnostics() const { return backend_.diagnostics(); }

RtspCameraSource::RtspCameraSource(CameraSettings settings)
    : settings_(settings), backend_(settings_) {}
bool RtspCameraSource::open(const std::string&) { return false; }
void RtspCameraSource::close() {}
bool RtspCameraSource::grab(core::Frame&) { return false; }
bool RtspCameraSource::isOpen() const { return false; }
core::CameraDiagnosticsSnapshot RtspCameraSource::diagnostics() const { return backend_.diagnostics(); }

IpCameraSource::IpCameraSource(CameraSettings settings)
    : settings_(settings), backend_(settings_) {}
bool IpCameraSource::open(const std::string&) { return false; }
void IpCameraSource::close() {}
bool IpCameraSource::grab(core::Frame&) { return false; }
bool IpCameraSource::isOpen() const { return false; }
core::CameraDiagnosticsSnapshot IpCameraSource::diagnostics() const { return backend_.diagnostics(); }

#endif

}  // namespace csx::camera
