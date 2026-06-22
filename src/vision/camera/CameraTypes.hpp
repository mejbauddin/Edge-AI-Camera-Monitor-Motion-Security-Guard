#pragma once

#include "types/Enums.hpp"
#include "types/Frame.hpp"

#include <cstdint>
#include <string>

namespace csx::camera {

struct CameraSettings {
    std::string cameraId{"primary"};
    std::uint32_t width{1280};
    std::uint32_t height{720};
    std::uint32_t targetFps{60};
    std::uint32_t bufferDepth{3};
    std::uint32_t reconnectIntervalMs{2000};
    std::uint32_t maxReconnectAttempts{0};
};

struct CameraSourceInfo {
    core::CameraSourceType type{core::CameraSourceType::Usb};
    std::string uri;
    int deviceIndex{0};
};

[[nodiscard]] CameraSourceInfo parseSourceUri(const std::string& sourceUri);

}  // namespace csx::camera
