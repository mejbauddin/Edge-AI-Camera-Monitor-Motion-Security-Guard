#pragma once

#include <cstdint>
#include <vector>

namespace csx::motion {

class MorphologyFilter {
public:
    explicit MorphologyFilter(int kernelSize = 3, int iterations = 1);

    void open(std::vector<std::uint8_t>& mask, std::uint32_t width, std::uint32_t height) const;
    void close(std::vector<std::uint8_t>& mask, std::uint32_t width, std::uint32_t height) const;

private:
    void erode(std::vector<std::uint8_t>& mask, std::uint32_t width, std::uint32_t height) const;
    void dilate(std::vector<std::uint8_t>& mask, std::uint32_t width, std::uint32_t height) const;

    int kernelSize_;
    int iterations_;
};

}  // namespace csx::motion
