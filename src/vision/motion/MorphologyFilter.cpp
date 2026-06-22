#include "MorphologyFilter.hpp"

#include <algorithm>

namespace csx::motion {

namespace {

constexpr std::uint8_t kForeground = 255;

}  // namespace

MorphologyFilter::MorphologyFilter(const int kernelSize, const int iterations)
    : kernelSize_(std::max(1, kernelSize)), iterations_(std::max(1, iterations)) {}

void MorphologyFilter::erode(std::vector<std::uint8_t>& mask, const std::uint32_t width,
                             const std::uint32_t height) const {
    const auto source = mask;
    const int radius = kernelSize_ / 2;

    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            bool keep = true;
            for (int ky = -radius; ky <= radius && keep; ++ky) {
                for (int kx = -radius; kx <= radius; ++kx) {
                    const auto nx = static_cast<int>(x) + kx;
                    const auto ny = static_cast<int>(y) + ky;
                    if (nx < 0 || ny < 0 || nx >= static_cast<int>(width) ||
                        ny >= static_cast<int>(height)) {
                        keep = false;
                        break;
                    }
                    if (source[static_cast<std::size_t>(ny) * width + static_cast<std::size_t>(nx)] <
                        kForeground) {
                        keep = false;
                        break;
                    }
                }
            }
            mask[static_cast<std::size_t>(y) * width + x] = keep ? kForeground : 0;
        }
    }
}

void MorphologyFilter::dilate(std::vector<std::uint8_t>& mask, const std::uint32_t width,
                              const std::uint32_t height) const {
    const auto source = mask;
    const int radius = kernelSize_ / 2;

    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            bool set = false;
            for (int ky = -radius; ky <= radius && !set; ++ky) {
                for (int kx = -radius; kx <= radius; ++kx) {
                    const auto nx = static_cast<int>(x) + kx;
                    const auto ny = static_cast<int>(y) + ky;
                    if (nx < 0 || ny < 0 || nx >= static_cast<int>(width) ||
                        ny >= static_cast<int>(height)) {
                        continue;
                    }
                    if (source[static_cast<std::size_t>(ny) * width + static_cast<std::size_t>(nx)] >=
                        kForeground) {
                        set = true;
                        break;
                    }
                }
            }
            mask[static_cast<std::size_t>(y) * width + x] = set ? kForeground : 0;
        }
    }
}

void MorphologyFilter::open(std::vector<std::uint8_t>& mask, const std::uint32_t width,
                            const std::uint32_t height) const {
    for (int i = 0; i < iterations_; ++i) {
        erode(mask, width, height);
        dilate(mask, width, height);
    }
}

void MorphologyFilter::close(std::vector<std::uint8_t>& mask, const std::uint32_t width,
                             const std::uint32_t height) const {
    for (int i = 0; i < iterations_; ++i) {
        dilate(mask, width, height);
        erode(mask, width, height);
    }
}

}  // namespace csx::motion
