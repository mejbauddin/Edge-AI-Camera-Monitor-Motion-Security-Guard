#pragma once

#include "CameraTypes.hpp"
#include "interfaces/Interfaces.hpp"

#include <atomic>
#include <cstdint>
#include <string>

namespace csx::camera {

class SyntheticCameraSource final : public core::ICameraSource {
public:
    explicit SyntheticCameraSource(CameraSettings settings = {});

    bool open(const std::string& sourceUri) override;
    void close() override;
    bool grab(core::Frame& outFrame) override;
    [[nodiscard]] bool isOpen() const override;
    [[nodiscard]] core::CameraDiagnosticsSnapshot diagnostics() const override;

private:
    CameraSettings settings_;
    std::atomic<bool> open_{false};
    std::uint64_t sequence_{0};
    core::CameraDiagnosticsSnapshot diagnostics_;
};

}  // namespace csx::camera
