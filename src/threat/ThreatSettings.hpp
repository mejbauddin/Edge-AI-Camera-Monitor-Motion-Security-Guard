#pragma once

#include "types/Enums.hpp"

#include <cstdint>
#include <string>

namespace csx::threat {

struct ThreatWeights {
    float motion{0.15F};
    float track{0.20F};
    float face{0.30F};
    float behavior{0.25F};
    float context{0.10F};
};

struct ThreatLevelThresholds {
    float greenMax{20.0F};
    float yellowMax{40.0F};
    float orangeMax{60.0F};
    float redMax{80.0F};
};

struct ThreatSettings {
    bool enabled{true};
    ThreatWeights weights{};
    ThreatLevelThresholds levels{};
    core::ThreatLevel autoRecordLevel{core::ThreatLevel::Orange};

    std::uint32_t nightHourStart{22};
    std::uint32_t nightHourEnd{6};
    float nightContextBoost{25.0F};

    std::size_t historyCapacity{30};
    float deescalationRate{0.35F};
    float authorizedFaceReduction{15.0F};

    float foeFaceScore{95.0F};
    float unknownFaceScore{50.0F};
    float authorizedFaceScore{5.0F};
};

[[nodiscard]] ThreatSettings defaultThreatSettings();

[[nodiscard]] core::ThreatLevel threatLevelFromName(const std::string& name);

}  // namespace csx::threat
