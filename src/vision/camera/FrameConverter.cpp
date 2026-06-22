#include "FrameConverter.hpp"

#include "CvHelpers.hpp"

#include <chrono>
#include <cstring>

namespace csx::camera {

core::Frame makeBgrFrame(const std::uint8_t* data, const std::uint32_t width,
                         const std::uint32_t height, const std::uint64_t sequence,
                         const std::string& cameraId) {
    core::Frame frame;
    frame.sequence = sequence;
    frame.cameraId = cameraId;
    frame.width = width;
    frame.height = height;
    frame.captureTime = std::chrono::steady_clock::now();

    const auto byteCount = csx::utils::bgrBufferSize(width, height);
    auto buffer = std::make_shared<std::vector<std::uint8_t>>(byteCount);
    if (data != nullptr && byteCount > 0) {
        std::memcpy(buffer->data(), data, byteCount);
    }
    frame.bgrData = std::move(buffer);
    return frame;
}

core::Frame makeSyntheticFrame(const std::uint32_t width, const std::uint32_t height,
                               const std::uint64_t sequence, const std::string& cameraId) {
    const auto byteCount = csx::utils::bgrBufferSize(width, height);
    auto buffer = std::make_shared<std::vector<std::uint8_t>>(byteCount);

    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            const auto index = (static_cast<std::size_t>(y) * width + x) * 3U;
            (*buffer)[index + 0] = static_cast<std::uint8_t>((x + sequence) % 256);
            (*buffer)[index + 1] = static_cast<std::uint8_t>((y + sequence) % 256);
            (*buffer)[index + 2] = static_cast<std::uint8_t>((x + y + sequence) % 256);
        }
    }

    core::Frame frame;
    frame.sequence = sequence;
    frame.cameraId = cameraId;
    frame.width = width;
    frame.height = height;
    frame.captureTime = std::chrono::steady_clock::now();
    frame.bgrData = std::move(buffer);
    frame.diagnostics.state = core::CameraState::Streaming;
    frame.diagnostics.width = width;
    frame.diagnostics.height = height;
    return frame;
}

}  // namespace csx::camera
