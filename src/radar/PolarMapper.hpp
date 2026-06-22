#pragma once

#include "types/Frame.hpp"
#include "types/Enums.hpp"

#include <cmath>
#include <cstdint>

namespace csx::radar {

// ──────────────────────────────────────────────────────────────────────────────
// PolarMapper — converts Cartesian camera coordinates to polar radar coordinates
// ──────────────────────────────────────────────────────────────────────────────
class PolarMapper {
public:
    struct PolarCoord {
        float distance;  // 0.0 to 1.0 (normalized to radar radius)
        float angle;     // radians, 0 at right, clockwise
    };

    struct RadarBlip {
        PolarCoord coord;
        core::IdentityClassification classification;
        core::ThreatLevel threatLevel;
        std::uint32_t trackId;
        float confidence;
        bool isPrediction;  // true for Kalman-predicted ghost dots
    };

    explicit PolarMapper(float maxRangeMeters = 10.0F);
    ~PolarMapper() = default;

    // Convert camera frame center (0,0 at top-left) to polar
    [[nodiscard]] PolarCoord cartesianToPolar(float x, float y, 
                                              float frameWidth, float frameHeight) const;

    // Convert a track to radar blip
    [[nodiscard]] RadarBlip trackToBlip(const core::Track& track,
                                        const core::FaceMatch* face,
                                        const core::ThreatAssessment& assessment,
                                        bool isPrediction = false) const;

    void setMaxRange(float meters) noexcept { maxRangeMeters_ = meters; }
    [[nodiscard]] float maxRange() const noexcept { return maxRangeMeters_; }

private:
    float maxRangeMeters_;
    // Assume camera FOV of 90 degrees horizontally for radar mapping
    static constexpr float HORIZONTAL_FOV_DEG = 90.0F;
    static constexpr float HORIZONTAL_FOV_RAD = HORIZONTAL_FOV_DEG * 3.14159265F / 180.0F;
};

}  // namespace csx::radar
