#pragma once

#include "types/Frame.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace csx::behavior {

struct RestrictedZone {
    std::string id;
    std::vector<core::Point2f> polygon;
    bool restricted{true};
};

struct BehaviorSettings {
    bool enabled{true};

    std::vector<RestrictedZone> zones;

    std::uint32_t loiteringThresholdMs{8000};
    float loiteringMaxSpeed{15.0F};
    float loiteringRadiusPx{40.0F};

    float runningSpeedThreshold{80.0F};
    float fallingAccelerationThreshold{-50.0F};
    float unusualVelocityZScore{2.5F};

    std::uint32_t baselineLearningFrames{120};
    float baselineAlpha{0.05F};

    float tamperBrightnessDelta{0.35F};
    float tamperBlurDropRatio{0.45F};
    std::uint32_t idleCameraMs{5000};

    std::uint32_t nightHourStart{22};
    std::uint32_t nightHourEnd{6};

    float minAnomalyScore{0.25F};
    float intrusionScore{0.85F};
    float loiteringScore{0.70F};
    float runningScore{0.55F};
    float unusualMovementScore{0.60F};
    float tamperScore{0.90F};
    float idleCameraScore{0.40F};
    float nightActivityScore{0.50F};
};

[[nodiscard]] BehaviorSettings defaultBehaviorSettings(std::uint32_t frameWidth = 640,
                                                       std::uint32_t frameHeight = 480);

}  // namespace csx::behavior
