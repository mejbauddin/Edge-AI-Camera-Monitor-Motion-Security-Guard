#include "RadarModel.hpp"

namespace csx::radar {

RadarModel::RadarModel(RadarSettings settings)
    : settings_(std::move(settings))
    , mapper_(settings_.maxRangeMeters)
    , sweep_(settings_.sweepRpm)
{
    engineHealth_.status = core::EngineStatus::Online;
}

void RadarModel::updateTracks(const std::vector<core::Track>& tracks,
                               const std::vector<core::FaceMatch>& faces,
                               const core::ThreatAssessment& assessment)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    blips_.clear();
    
    for (const auto& track : tracks) {
        if (!track.active) continue;
        
        // Find matching face
        const core::FaceMatch* face = nullptr;
        for (const auto& fm : faces) {
            if (fm.trackId == track.id) {
                face = &fm;
                break;
            }
        }
        
        // Add main blip
        blips_.push_back(mapper_.trackToBlip(track, face, assessment, false));
        
        // Add prediction ghost if enabled
        if (settings_.showPredictionGhosts) {
            // Create a temporary track with predicted position
            core::Track predTrack = track;
            predTrack.center = track.predictedPosition;
            blips_.push_back(mapper_.trackToBlip(predTrack, face, assessment, true));
        }
    }
    
    updateCount_.fetch_add(1, std::memory_order_relaxed);
    engineHealth_.processedFrames = updateCount_.load(std::memory_order_relaxed);
}

std::vector<PolarMapper::RadarBlip> RadarModel::getBlips() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return blips_;
}

float RadarModel::getSweepAngle() const {
    return sweep_.currentAngle();
}

core::EngineHealth RadarModel::health() const {
    return engineHealth_;
}

void RadarModel::setSweepRpm(float rpm) {
    sweep_.setRpm(rpm);
    settings_.sweepRpm = rpm;
}

void RadarModel::update() {
    sweep_.update();
}

}  // namespace csx::radar
