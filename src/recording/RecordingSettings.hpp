#pragma once

#include "types/Enums.hpp"

#include <cstdint>
#include <string>

namespace csx::recording {

struct RecordingSettings {
    bool enabled{true};
    std::uint32_t preBufferSeconds{5};
    std::uint32_t postBufferSeconds{10};
    std::uint32_t assumedFps{30};
    std::string codec{"h264"};
    std::string outputDirectory{"recordings"};
    double maxStorageGb{50.0};
    core::ThreatLevel autoRecordLevel{core::ThreatLevel::Orange};
    bool captureSnapshotOnTrigger{true};
};

[[nodiscard]] RecordingSettings defaultRecordingSettings();

}  // namespace csx::recording
