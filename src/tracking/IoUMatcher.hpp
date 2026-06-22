#pragma once

#include "types/Frame.hpp"

namespace csx::tracking {

[[nodiscard]] float computeIoU(const core::Rect2f& a, const core::Rect2f& b) noexcept;

class IoUMatcher {
public:
    explicit IoUMatcher(float iouThreshold = 0.3F);

    void match(const std::vector<core::Rect2f>& detections,
               const std::vector<core::Rect2f>& trackBoxes,
               std::vector<int>& outDetectionToTrack,
               std::vector<int>& outUnmatchedDetections,
               std::vector<int>& outUnmatchedTracks) const;

private:
    float iouThreshold_;
};

}  // namespace csx::tracking
