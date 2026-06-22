#include "BlobAnalyzer.hpp"

#include <algorithm>
#include <array>
#include <queue>

namespace csx::motion {

namespace {

constexpr std::array<std::pair<int, int>, 4> kDirections = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

}  // namespace

BlobAnalyzer::BlobAnalyzer(const int minBlobArea) : minBlobArea_(std::max(1, minBlobArea)) {}

void BlobAnalyzer::analyze(const std::vector<std::uint8_t>& mask, const std::uint32_t maskWidth,
                           const std::uint32_t maskHeight, const float scaleX, const float scaleY,
                           std::vector<core::Rect2f>& outBlobs) const {
    outBlobs.clear();
    if (mask.empty() || maskWidth == 0 || maskHeight == 0) {
        return;
    }

    std::vector<std::uint8_t> visited(mask.size(), 0);

    for (std::uint32_t y = 0; y < maskHeight; ++y) {
        for (std::uint32_t x = 0; x < maskWidth; ++x) {
            const auto index = static_cast<std::size_t>(y) * maskWidth + x;
            if (mask[index] == 0 || visited[index] != 0) {
                continue;
            }

            std::uint32_t minX = x;
            std::uint32_t maxX = x;
            std::uint32_t minY = y;
            std::uint32_t maxY = y;
            std::size_t area = 0;

            std::queue<std::pair<std::uint32_t, std::uint32_t>> queue;
            queue.emplace(x, y);
            visited[index] = 1;

            while (!queue.empty()) {
                const auto [currentX, currentY] = queue.front();
                queue.pop();
                ++area;
                minX = std::min(minX, currentX);
                maxX = std::max(maxX, currentX);
                minY = std::min(minY, currentY);
                maxY = std::max(maxY, currentY);

                for (const auto& [dx, dy] : kDirections) {
                    const auto nx = static_cast<int>(currentX) + dx;
                    const auto ny = static_cast<int>(currentY) + dy;
                    if (nx < 0 || ny < 0 || nx >= static_cast<int>(maskWidth) ||
                        ny >= static_cast<int>(maskHeight)) {
                        continue;
                    }
                    const auto neighborIndex =
                        static_cast<std::size_t>(ny) * maskWidth + static_cast<std::uint32_t>(nx);
                    if (mask[neighborIndex] == 0 || visited[neighborIndex] != 0) {
                        continue;
                    }
                    visited[neighborIndex] = 1;
                    queue.emplace(static_cast<std::uint32_t>(nx), static_cast<std::uint32_t>(ny));
                }
            }

            if (static_cast<int>(area) < minBlobArea_) {
                continue;
            }

            core::Rect2f bbox;
            bbox.x = static_cast<float>(minX) * scaleX;
            bbox.y = static_cast<float>(minY) * scaleY;
            bbox.width = static_cast<float>(maxX - minX + 1U) * scaleX;
            bbox.height = static_cast<float>(maxY - minY + 1U) * scaleY;
            outBlobs.push_back(bbox);
        }
    }
}

}  // namespace csx::motion
