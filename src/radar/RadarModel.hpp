#pragma once

#include "PolarMapper.hpp"
#include "SweepController.hpp"
#include "interfaces/Interfaces.hpp"
#include "types/Frame.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

namespace csx::radar {

// ──────────────────────────────────────────────────────────────────────────────
// RadarSettings — mirrors config/default.json radar.* section
// ──────────────────────────────────────────────────────────────────────────────
struct RadarSettings {
    float sweepRpm{4.0F};
    float maxRangeMeters{10.0F};
    bool showPredictionGhosts{true};
    float ghostFadeDuration{2.0F};  // seconds
    int blipSize{4};
};

// Forward declaration
struct RadarBlip;

// ──────────────────────────────────────────────────────────────────────────────
// RadarModel — maintains radar state for QML binding
// ──────────────────────────────────────────────────────────────────────────────
class RadarModel {
public:
    explicit RadarModel(RadarSettings settings = {});
    ~RadarModel() = default;

    void updateTracks(const std::vector<core::Track>& tracks,
                      const std::vector<core::FaceMatch>& faces,
                      const core::ThreatAssessment& assessment);
    
    [[nodiscard]] std::vector<PolarMapper::RadarBlip> getBlips() const;
    [[nodiscard]] float getSweepAngle() const;
    [[nodiscard]] core::EngineHealth health() const;

    // Control
    void setSweepRpm(float rpm);
    void update();  // call this each frame to advance sweep

    [[nodiscard]] const RadarSettings& settings() const noexcept { return settings_; }

private:
    mutable std::mutex mutex_;
    RadarSettings settings_;
    
    PolarMapper mapper_;
    SweepController sweep_;
    
    std::vector<PolarMapper::RadarBlip> blips_;
    std::atomic<std::uint64_t> updateCount_{0};
    core::EngineHealth engineHealth_;
};

}  // namespace csx::radar
