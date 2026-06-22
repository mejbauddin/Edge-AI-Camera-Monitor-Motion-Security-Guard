#include "CameraTypes.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace csx::camera {

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

}  // namespace

CameraSourceInfo parseSourceUri(const std::string& sourceUri) {
    CameraSourceInfo info;
    info.uri = sourceUri;

    const auto lower = toLower(sourceUri);
    if (lower.rfind("usb:", 0) == 0) {
        info.type = core::CameraSourceType::Usb;
        info.deviceIndex = std::stoi(sourceUri.substr(4));
        return info;
    }

    if (lower.rfind("synthetic:", 0) == 0) {
        info.type = core::CameraSourceType::Usb;
        info.deviceIndex = std::stoi(sourceUri.substr(10));
        return info;
    }

    if (lower.rfind("rtsp://", 0) == 0) {
        info.type = core::CameraSourceType::Rtsp;
        return info;
    }

    if (lower.rfind("http://", 0) == 0 || lower.rfind("https://", 0) == 0 ||
        lower.rfind("ip://", 0) == 0) {
        info.type = core::CameraSourceType::Ip;
        if (lower.rfind("ip://", 0) == 0) {
            info.uri = "http://" + sourceUri.substr(5);
        }
        return info;
    }

    info.type = core::CameraSourceType::Usb;
    try {
        info.deviceIndex = std::stoi(sourceUri);
    } catch (const std::exception&) {
        info.deviceIndex = 0;
    }
    return info;
}

}  // namespace csx::camera
