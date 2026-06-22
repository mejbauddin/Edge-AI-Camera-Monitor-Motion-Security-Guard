#pragma once

#include "CameraTypes.hpp"
#include "interfaces/Interfaces.hpp"

#include <memory>
#include <mutex>
#include <string>

namespace csx::camera {

class OpenCvCaptureBackend {
public:
    explicit OpenCvCaptureBackend(CameraSettings settings);
    ~OpenCvCaptureBackend();

    bool openUsb(int deviceIndex);
    bool openStream(const std::string& url, bool preferFfmpeg);
    void close();
    [[nodiscard]] bool isOpen() const;
    bool read(core::Frame& outFrame);

    [[nodiscard]] core::CameraDiagnosticsSnapshot diagnostics() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    CameraSettings settings_;
    mutable std::mutex mutex_;
    core::CameraDiagnosticsSnapshot diagnostics_;
    std::uint64_t sequence_{0};
};

class UsbCameraSource final : public core::ICameraSource {
public:
    explicit UsbCameraSource(CameraSettings settings = {});

    bool open(const std::string& sourceUri) override;
    void close() override;
    bool grab(core::Frame& outFrame) override;
    [[nodiscard]] bool isOpen() const override;
    [[nodiscard]] core::CameraDiagnosticsSnapshot diagnostics() const override;

private:
    CameraSettings settings_;
    OpenCvCaptureBackend backend_;
};

class RtspCameraSource final : public core::ICameraSource {
public:
    explicit RtspCameraSource(CameraSettings settings = {});

    bool open(const std::string& sourceUri) override;
    void close() override;
    bool grab(core::Frame& outFrame) override;
    [[nodiscard]] bool isOpen() const override;
    [[nodiscard]] core::CameraDiagnosticsSnapshot diagnostics() const override;

private:
    CameraSettings settings_;
    OpenCvCaptureBackend backend_;
};

class IpCameraSource final : public core::ICameraSource {
public:
    explicit IpCameraSource(CameraSettings settings = {});

    bool open(const std::string& sourceUri) override;
    void close() override;
    bool grab(core::Frame& outFrame) override;
    [[nodiscard]] bool isOpen() const override;
    [[nodiscard]] core::CameraDiagnosticsSnapshot diagnostics() const override;

private:
    CameraSettings settings_;
    OpenCvCaptureBackend backend_;
};

}  // namespace csx::camera
