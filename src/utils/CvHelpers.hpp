#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

namespace csx::utils {

[[nodiscard]] constexpr std::size_t bgrStride(std::uint32_t width) noexcept {
    return static_cast<std::size_t>(width) * 3U;
}

[[nodiscard]] constexpr std::size_t bgrBufferSize(std::uint32_t width, std::uint32_t height) noexcept {
    return bgrStride(width) * static_cast<std::size_t>(height);
}

template <typename T>
[[nodiscard]] constexpr T clampValue(T value, T low, T high) noexcept {
    return value < low ? low : (value > high ? high : value);
}

void resizeBgrNearestNeighbor(const std::uint8_t* source, std::uint32_t sourceWidth,
                              std::uint32_t sourceHeight, std::uint8_t* destination,
                              std::uint32_t destinationWidth, std::uint32_t destinationHeight);

void fillBgr(std::uint8_t* buffer, std::uint32_t width, std::uint32_t height, std::uint8_t blue,
             std::uint8_t green, std::uint8_t red);

void copyBgrRegion(const std::uint8_t* source, std::uint32_t sourceWidth, std::uint32_t sourceHeight,
                   std::uint32_t x, std::uint32_t y, std::uint32_t regionWidth,
                   std::uint32_t regionHeight, std::vector<std::uint8_t>& outRegion);

std::mutex& openCvMutex();

}  // namespace csx::utils
