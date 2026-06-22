#include "IoUMatcher.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <vector>

namespace csx::tracking {

float computeIoU(const core::Rect2f& a, const core::Rect2f& b) noexcept {
    const float x1 = std::max(a.x, b.x);
    const float y1 = std::max(a.y, b.y);
    const float x2 = std::min(a.x + a.width, b.x + b.width);
    const float y2 = std::min(a.y + a.height, b.y + b.height);

    const float intersectionWidth = std::max(0.0F, x2 - x1);
    const float intersectionHeight = std::max(0.0F, y2 - y1);
    const float intersection = intersectionWidth * intersectionHeight;

    const float areaA = a.width * a.height;
    const float areaB = b.width * b.height;
    const float unionArea = areaA + areaB - intersection;
    if (unionArea <= 0.0F) {
        return 0.0F;
    }
    return intersection / unionArea;
}

IoUMatcher::IoUMatcher(const float iouThreshold) : iouThreshold_(iouThreshold) {}

void IoUMatcher::match(const std::vector<core::Rect2f>& detections,
                        const std::vector<core::Rect2f>& trackBoxes,
                        std::vector<int>& outDetectionToTrack,
                        std::vector<int>& outUnmatchedDetections,
                        std::vector<int>& outUnmatchedTracks) const {
    outDetectionToTrack.assign(detections.size(), -1);
    outUnmatchedDetections.clear();
    outUnmatchedTracks.clear();

    if (detections.empty()) {
        outUnmatchedTracks.resize(trackBoxes.size());
        std::iota(outUnmatchedTracks.begin(), outUnmatchedTracks.end(), 0);
        return;
    }

    if (trackBoxes.empty()) {
        outUnmatchedDetections.resize(detections.size());
        std::iota(outUnmatchedDetections.begin(), outUnmatchedDetections.end(), 0);
        return;
    }

    struct Candidate {
        int detectionIndex;
        int trackIndex;
        float iou;
    };

    std::vector<Candidate> candidates;
    candidates.reserve(detections.size() * trackBoxes.size());
    for (int detectionIndex = 0; detectionIndex < static_cast<int>(detections.size()); ++detectionIndex) {
        for (int trackIndex = 0; trackIndex < static_cast<int>(trackBoxes.size()); ++trackIndex) {
            const float iou = computeIoU(detections[static_cast<std::size_t>(detectionIndex)],
                                         trackBoxes[static_cast<std::size_t>(trackIndex)]);
            if (iou >= iouThreshold_) {
                candidates.push_back({detectionIndex, trackIndex, iou});
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& lhs, const Candidate& rhs) { return lhs.iou > rhs.iou; });

    std::vector<bool> detectionUsed(detections.size(), false);
    std::vector<bool> trackUsed(trackBoxes.size(), false);

    for (const auto& candidate : candidates) {
        if (detectionUsed[static_cast<std::size_t>(candidate.detectionIndex)] ||
            trackUsed[static_cast<std::size_t>(candidate.trackIndex)]) {
            continue;
        }
        outDetectionToTrack[static_cast<std::size_t>(candidate.detectionIndex)] = candidate.trackIndex;
        detectionUsed[static_cast<std::size_t>(candidate.detectionIndex)] = true;
        trackUsed[static_cast<std::size_t>(candidate.trackIndex)] = true;
    }

    for (int detectionIndex = 0; detectionIndex < static_cast<int>(detections.size()); ++detectionIndex) {
        if (!detectionUsed[static_cast<std::size_t>(detectionIndex)]) {
            outUnmatchedDetections.push_back(detectionIndex);
        }
    }

    for (int trackIndex = 0; trackIndex < static_cast<int>(trackBoxes.size()); ++trackIndex) {
        if (!trackUsed[static_cast<std::size_t>(trackIndex)]) {
            outUnmatchedTracks.push_back(trackIndex);
        }
    }
}

}  // namespace csx::tracking
