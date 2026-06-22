#include "PolarMapper.hpp"

#include <algorithm>

namespace csx::radar {

PolarMapper::PolarMapper(float maxRangeMeters) : maxRangeMeters_(maxRangeMeters) {}

PolarMapper::PolarCoord PolarMapper::cartesianToPolar(float x, float y,
                                                       float frameWidth, float frameHeight) const
{
    // Center coordinates (0,0 at center of frame)
    const float cx = frameWidth / 2.0F;
    const float cy = frameHeight / 2.0F;
    const float dx = x - cx;
    const float dy = y - cy;

    // Calculate distance (normalized to max range)
    const float pixelDistance = std::sqrt(dx * dx + dy * dy);
    const float maxPixelDistance = std::sqrt(cx * cx + cy * cy);
    const float normalizedDistance = std::min(1.0F, pixelDistance / maxPixelDistance);

    // Calculate angle (0 at right, clockwise)
    // atan2 returns angle from positive X axis, counter-clockwise
    float angle = std::atan2(dy, dx);
    
    // Convert to clockwise from right (0 at 3 o'clock)
    angle = -angle; // flip to clockwise
    if (angle < 0.0F) {
        angle += 2.0F * 3.14159265F;
    }

    return {normalizedDistance, angle};
}

PolarMapper::RadarBlip PolarMapper::trackToBlip(const core::Track& track,
                                                 const core::FaceMatch* face,
                                                 const core::ThreatAssessment& assessment,
                                                 bool isPrediction) const
{
    PolarCoord coord = cartesianToPolar(track.center.x, track.center.y,
                                         static_cast<float>(track.bbox.width),
                                         static_cast<float>(track.bbox.height));

    // Determine classification from face match if available
    auto classification = core::IdentityClassification::Unknown;
    if (face) {
        classification = face->classification;
    }

    // Determine threat level from assessment
    auto threatLevel = core::ThreatLevel::Green;
    // Check if this track contributes to elevated threat
    if (assessment.level >= core::ThreatLevel::Yellow) {
        threatLevel = assessment.level;
    }

    return {coord, classification, threatLevel, track.id, track.confidence, isPrediction};
}

}  // namespace csx::radar
