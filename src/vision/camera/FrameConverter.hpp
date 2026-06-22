#pragma once

#include "types/Frame.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace csx::camera {

[[nodiscard]] core::Frame makeBgrFrame(const std::uint8_t* data, std::uint32_t width,
                                       std::uint32_t height, std::uint64_t sequence,
                                       const std::string& cameraId);

[[nodiscard]] core::Frame makeSyntheticFrame(std::uint32_t width, std::uint32_t height,
                                             std::uint64_t sequence, const std::string& cameraId);

}  // namespace csx::camera
