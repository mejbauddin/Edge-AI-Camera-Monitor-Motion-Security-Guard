#include "CameraSourceFactory.hpp"

#include "OpenCvCameraSources.hpp"
#include "SyntheticCameraSource.hpp"

#include <algorithm>
#include <cctype>

namespace csx::camera {

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](const unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

}  // namespace

std::unique_ptr<core::ICameraSource> createCameraSource(const std::string& sourceUri,
                                                        CameraSettings settings) {
    const auto lower = toLower(sourceUri);

    if (lower.rfind("synthetic:", 0) == 0) {
        return std::make_unique<SyntheticCameraSource>(settings);
    }

    const auto info = parseSourceUri(sourceUri);
    switch (info.type) {
        case core::CameraSourceType::Usb:
            return std::make_unique<UsbCameraSource>(settings);
        case core::CameraSourceType::Rtsp:
            return std::make_unique<RtspCameraSource>(settings);
        case core::CameraSourceType::Ip:
            return std::make_unique<IpCameraSource>(settings);
    }

    return std::make_unique<SyntheticCameraSource>(settings);
}

}  // namespace csx::camera
