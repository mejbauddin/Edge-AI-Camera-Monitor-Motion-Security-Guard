#pragma once

#include "types/Enums.hpp"
#include "types/Frame.hpp"

#include <chrono>
#include <string>

namespace csx::alerts {

// ──────────────────────────────────────────────────────────────────────────────
// AlertPolicy — defines when alerts should trigger based on threat level
// ──────────────────────────────────────────────────────────────────────────────
struct AlertPolicy {
    core::ThreatLevel minTriggerLevel{core::ThreatLevel::Yellow};
    bool voiceEnabled{true};
    bool visualEnabled{true};
    bool recordingTrigger{true};
    int cooldownMs{5000};  // minimum time between alerts of same type
    
    // Voice phrases for each threat level
    std::string voiceGreen;
    std::string voiceYellow{"Threat level elevated"};
    std::string voiceOrange{"Warning - potential threat detected"};
    std::string voiceRed{"Alert - high threat level"};
    std::string voiceCritical{"Critical threat - immediate attention required"};
};

// ──────────────────────────────────────────────────────────────────────────────
// Alert — represents a single alert event
// ──────────────────────────────────────────────────────────────────────────────
struct Alert {
    std::string id;
    std::string message;
    core::ThreatLevel level;
    std::chrono::steady_clock::time_point timestamp;
    std::uint32_t trackId;
    bool acknowledged{false};
    
    Alert(std::string msg, core::ThreatLevel lvl, std::uint32_t trackId = 0)
        : message(std::move(msg))
        , level(lvl)
        , timestamp(std::chrono::steady_clock::now())
        , trackId(trackId)
    {}
};

}  // namespace csx::alerts
