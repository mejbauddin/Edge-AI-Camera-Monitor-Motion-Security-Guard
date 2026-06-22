#pragma once

#include "BehaviorSettings.hpp"
#include "types/Frame.hpp"

#include <cmath>
#include <vector>

namespace csx::behavior {

[[nodiscard]] inline float distance(const core::Point2f& a, const core::Point2f& b) noexcept {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

[[nodiscard]] inline float velocityMagnitude(const core::Point2f& velocity) noexcept {
    return distance({0.0F, 0.0F}, velocity);
}

[[nodiscard]] bool pointInPolygon(const core::Point2f& point,
                                  const std::vector<core::Point2f>& polygon);

[[nodiscard]] bool pointInAnyZone(const core::Point2f& point,
                                  const std::vector<RestrictedZone>& zones,
                                  bool restrictedOnly = true);

}  // namespace csx::behavior
