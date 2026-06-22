#pragma once

#include "types/Frame.hpp"

#include <cstdint>
#include <vector>

namespace csx::motion {

struct BlobInfo {
    core::Rect2f bbox;
    float area{0.0F};
    core::Point2f center;
};

class BlobAnalyzer {
public:
    explicit BlobAnalyzer(int minBlobArea = 400);

    void analyze(const std::vector<std::uint8_t>& mask, std::uint32_t maskWidth,
                 std::uint32_t maskHeight, float scaleX, float scaleY,
                 std::vector<core::Rect2f>& outBlobs) const;

private:
    int minBlobArea_;
};

}  // namespace csx::motion
