#pragma once

#include "IoUMatcher.hpp"
#include "TrackManager.hpp"
#include "TrackingSettings.hpp"
#include "interfaces/Interfaces.hpp"

#include <memory>

namespace csx::tracking {

class ObjectTracker final : public core::ITracker {
public:
    explicit ObjectTracker(TrackingSettings settings = {});

    void update(const core::Frame& frame, const std::vector<core::Rect2f>& detections,
                std::vector<core::Track>& outTracks) override;
    [[nodiscard]] core::EngineHealth health() const override;

    void setSettings(const TrackingSettings& settings);

private:
    TrackingSettings settings_;
    TrackManager trackManager_;
    IoUMatcher matcher_;
    core::EngineHealth health_;
    std::uint64_t processedFrames_{0};
    double lastLatencyMs_{0.0};
};

std::shared_ptr<core::ITracker> createObjectTracker(TrackingSettings settings = {});

}  // namespace csx::tracking
