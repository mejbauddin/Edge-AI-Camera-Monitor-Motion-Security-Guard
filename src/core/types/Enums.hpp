#pragma once

#include <cstdint>
#include <string>

namespace csx::core {

enum class ThreatLevel : std::uint8_t {
    Green    = 0,
    Yellow   = 1,
    Orange   = 2,
    Red      = 3,
    Critical = 4
};

enum class CameraState : std::uint8_t {
    Disconnected = 0,
    Connecting   = 1,
    Streaming    = 2,
    Reconnecting = 3,
    Error        = 4
};

enum class CameraSourceType : std::uint8_t {
    Usb  = 0,
    Rtsp = 1,
    Ip   = 2
};

enum class EngineStatus : std::uint8_t {
    Offline   = 0,
    Starting  = 1,
    Online    = 2,
    Degraded  = 3,
    Fault     = 4
};

enum class BehaviorAnomalyType : std::uint8_t {
    None                    = 0,
    Intrusion               = 1,
    Loitering               = 2,
    Running                 = 3,
    Falling                 = 4,
    NightActivity           = 5,
    UnusualMovement         = 6,
    CameraTampering         = 7,
    IdleCamera              = 8,
    UnexpectedObject        = 9,
    UnexpectedDisappearance = 10
};

enum class IdentityClassification : std::uint8_t {
    Unknown    = 0,
    Authorized = 1,
    Foe        = 2
};

enum class DefconLevel : std::uint8_t {
    Defcon5 = 5,
    Defcon4 = 4,
    Defcon3 = 3,
    Defcon2 = 2,
    Defcon1 = 1
};

[[nodiscard]] std::string toString(ThreatLevel level);
[[nodiscard]] std::string toString(CameraState state);
[[nodiscard]] std::string toString(EngineStatus status);
[[nodiscard]] std::string toString(BehaviorAnomalyType type);
[[nodiscard]] std::string toString(IdentityClassification classification);
[[nodiscard]] std::string toString(DefconLevel level);

[[nodiscard]] ThreatLevel threatLevelFromScore(float score, float greenMax, float yellowMax,
                                               float orangeMax, float redMax);
[[nodiscard]] DefconLevel defconFromThreatLevel(ThreatLevel level);

}  // namespace csx::core
