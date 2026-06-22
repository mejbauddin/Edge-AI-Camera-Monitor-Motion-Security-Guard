#include "CvHelpers.hpp"

#include <cstring>
#include <mutex>

namespace csx::utils {

void resizeBgrNearestNeighbor(const std::uint8_t* source, const std::uint32_t sourceWidth,
                              const std::uint32_t sourceHeight, std::uint8_t* destination,
                              const std::uint32_t destinationWidth,
                              const std::uint32_t destinationHeight) {
    if (source == nullptr || destination == nullptr || sourceWidth == 0 || sourceHeight == 0 ||
        destinationWidth == 0 || destinationHeight == 0) {
        return;
    }

    const float xRatio = static_cast<float>(sourceWidth) / static_cast<float>(destinationWidth);
    const float yRatio = static_cast<float>(sourceHeight) / static_cast<float>(destinationHeight);

    for (std::uint32_t destY = 0; destY < destinationHeight; ++destY) {
        const auto sourceY = static_cast<std::uint32_t>(destY * yRatio);
        const auto sourceRow = source + static_cast<std::size_t>(sourceY) * bgrStride(sourceWidth);
        auto destRow = destination + static_cast<std::size_t>(destY) * bgrStride(destinationWidth);

        for (std::uint32_t destX = 0; destX < destinationWidth; ++destX) {
            const auto sourceX = static_cast<std::uint32_t>(destX * xRatio);
            const auto* pixel = sourceRow + static_cast<std::size_t>(sourceX) * 3U;
            destRow[0] = pixel[0];
            destRow[1] = pixel[1];
            destRow[2] = pixel[2];
            destRow += 3;
        }
    }
}

void fillBgr(std::uint8_t* buffer, const std::uint32_t width, const std::uint32_t height,
             const std::uint8_t blue, const std::uint8_t green, const std::uint8_t red) {
    if (buffer == nullptr || width == 0 || height == 0) {
        return;
    }

    const std::uint8_t pixel[3] = {blue, green, red};
    const auto stride = bgrStride(width);
    for (std::uint32_t row = 0; row < height; ++row) {
        auto* rowPtr = buffer + static_cast<std::size_t>(row) * stride;
        for (std::uint32_t col = 0; col < width; ++col) {
            std::memcpy(rowPtr + static_cast<std::size_t>(col) * 3U, pixel, 3);
        }
    }
}

void copyBgrRegion(const std::uint8_t* source, const std::uint32_t sourceWidth,
                   const std::uint32_t sourceHeight, const std::uint32_t x, const std::uint32_t y,
                   const std::uint32_t regionWidth, const std::uint32_t regionHeight,
                   std::vector<std::uint8_t>& outRegion) {
    if (source == nullptr || regionWidth == 0 || regionHeight == 0) {
        outRegion.clear();
        return;
    }

    if (x + regionWidth > sourceWidth || y + regionHeight > sourceHeight) {
        outRegion.clear();
        return;
    }

    outRegion.resize(bgrBufferSize(regionWidth, regionHeight));
    for (std::uint32_t row = 0; row < regionHeight; ++row) {
        const auto* src = source + bgrStride(sourceWidth) * static_cast<std::size_t>(y + row) +
                          static_cast<std::size_t>(x) * 3U;
        auto* dst = outRegion.data() + bgrStride(regionWidth) * static_cast<std::size_t>(row);
        std::memcpy(dst, src, bgrStride(regionWidth));
    }
}

std::mutex& openCvMutex() {
    static std::mutex mutex;
    return mutex;
}

}  // namespace csx::utils
