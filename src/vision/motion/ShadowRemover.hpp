#pragma once

#include <cstdint>
#include <vector>

namespace csx::motion {

class ShadowRemover {
public:
    void apply(const std::uint8_t* bgr, std::vector<std::uint8_t>& mask, std::uint32_t width,
               std::uint32_t height) const;

private:
    [[nodiscard]] static float brightness(const std::uint8_t* pixel) noexcept;
};

}  // namespace csx::motion
