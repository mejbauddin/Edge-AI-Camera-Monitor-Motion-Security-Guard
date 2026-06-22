#include "ShadowRemover.hpp"

namespace csx::motion {

float ShadowRemover::brightness(const std::uint8_t* pixel) noexcept {
    return 0.114F * static_cast<float>(pixel[0]) + 0.587F * static_cast<float>(pixel[1]) +
           0.299F * static_cast<float>(pixel[2]);
}

void ShadowRemover::apply(const std::uint8_t* bgr, std::vector<std::uint8_t>& mask,
                          const std::uint32_t width, const std::uint32_t height) const {
    if (bgr == nullptr || mask.size() != static_cast<std::size_t>(width) * height) {
        return;
    }

    float averageForeground = 0.0F;
    std::size_t foregroundCount = 0;
    for (std::size_t i = 0; i < mask.size(); ++i) {
        if (mask[i] == 0) {
            continue;
        }
        averageForeground += brightness(bgr + i * 3U);
        ++foregroundCount;
    }

    if (foregroundCount == 0) {
        return;
    }

    averageForeground /= static_cast<float>(foregroundCount);
    const float shadowThreshold = averageForeground * 0.55F;

    for (std::size_t i = 0; i < mask.size(); ++i) {
        if (mask[i] == 0) {
            continue;
        }
        if (brightness(bgr + i * 3U) < shadowThreshold) {
            mask[i] = 0;
        }
    }
}

}  // namespace csx::motion
