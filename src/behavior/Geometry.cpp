#include "Geometry.hpp"

#include "BehaviorSettings.hpp"

namespace csx::behavior {

bool pointInPolygon(const core::Point2f& point, const std::vector<core::Point2f>& polygon) {
    if (polygon.size() < 3U) {
        return false;
    }

    bool inside = false;
    for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const auto& pi = polygon[i];
        const auto& pj = polygon[j];
        const bool intersects =
            ((pi.y > point.y) != (pj.y > point.y)) &&
            (point.x < (pj.x - pi.x) * (point.y - pi.y) / (pj.y - pi.y + 1e-6F) + pi.x);
        if (intersects) {
            inside = !inside;
        }
    }
    return inside;
}

bool pointInAnyZone(const core::Point2f& point, const std::vector<RestrictedZone>& zones,
                    const bool restrictedOnly) {
    for (const auto& zone : zones) {
        if (restrictedOnly && !zone.restricted) {
            continue;
        }
        if (pointInPolygon(point, zone.polygon)) {
            return true;
        }
    }
    return false;
}

}  // namespace csx::behavior
