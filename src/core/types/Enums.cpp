#include "types/Enums.hpp"

namespace csx::core {

std::string toString(const ThreatLevel level) {
    switch (level) {
        case ThreatLevel::Green:    return "GREEN";
        case ThreatLevel::Yellow:   return "YELLOW";
        case ThreatLevel::Orange:   return "ORANGE";
        case ThreatLevel::Red:      return "RED";
        case ThreatLevel::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

std::string toString(const CameraState state) {
    switch (state) {
        case CameraState::Disconnected: return "DISCONNECTED";
        case CameraState::Connecting:   return "CONNECTING";
        case CameraState::Streaming:    return "STREAMING";
        case CameraState::Reconnecting: return "RECONNECTING";
        case CameraState::Error:        return "ERROR";
    }
    return "UNKNOWN";
}

std::string toString(const EngineStatus status) {
    switch (status) {
        case EngineStatus::Offline:  return "OFFLINE";
        case EngineStatus::Starting: return "STARTING";
        case EngineStatus::Online:   return "ONLINE";
        case EngineStatus::Degraded:   return "DEGRADED";
        case EngineStatus::Fault:      return "FAULT";
    }
    return "UNKNOWN";
}

std::string toString(const BehaviorAnomalyType type) {
    switch (type) {
        case BehaviorAnomalyType::None:                    return "NONE";
        case BehaviorAnomalyType::Intrusion:               return "INTRUSION";
        case BehaviorAnomalyType::Loitering:               return "LOITERING";
        case BehaviorAnomalyType::Running:                 return "RUNNING";
        case BehaviorAnomalyType::Falling:                 return "FALLING";
        case BehaviorAnomalyType::NightActivity:           return "NIGHT_ACTIVITY";
        case BehaviorAnomalyType::UnusualMovement:         return "UNUSUAL_MOVEMENT";
        case BehaviorAnomalyType::CameraTampering:         return "CAMERA_TAMPERING";
        case BehaviorAnomalyType::IdleCamera:              return "IDLE_CAMERA";
        case BehaviorAnomalyType::UnexpectedObject:        return "UNEXPECTED_OBJECT";
        case BehaviorAnomalyType::UnexpectedDisappearance: return "UNEXPECTED_DISAPPEARANCE";
    }
    return "UNKNOWN";
}

std::string toString(const IdentityClassification classification) {
    switch (classification) {
        case IdentityClassification::Unknown:    return "UNKNOWN";
        case IdentityClassification::Authorized: return "AUTHORIZED";
        case IdentityClassification::Foe:        return "FOE";
    }
    return "UNKNOWN";
}

std::string toString(const DefconLevel level) {
    switch (level) {
        case DefconLevel::Defcon5: return "DEFCON 5";
        case DefconLevel::Defcon4: return "DEFCON 4";
        case DefconLevel::Defcon3: return "DEFCON 3";
        case DefconLevel::Defcon2: return "DEFCON 2";
        case DefconLevel::Defcon1: return "DEFCON 1";
    }
    return "DEFCON ?";
}

ThreatLevel threatLevelFromScore(const float score, const float greenMax, const float yellowMax,
                                 const float orangeMax, const float redMax) {
    if (score <= greenMax)   return ThreatLevel::Green;
    if (score <= yellowMax)  return ThreatLevel::Yellow;
    if (score <= orangeMax)  return ThreatLevel::Orange;
    if (score <= redMax)     return ThreatLevel::Red;
    return ThreatLevel::Critical;
}

DefconLevel defconFromThreatLevel(const ThreatLevel level) {
    switch (level) {
        case ThreatLevel::Green:    return DefconLevel::Defcon5;
        case ThreatLevel::Yellow:   return DefconLevel::Defcon4;
        case ThreatLevel::Orange:   return DefconLevel::Defcon3;
        case ThreatLevel::Red:      return DefconLevel::Defcon2;
        case ThreatLevel::Critical: return DefconLevel::Defcon1;
    }
    return DefconLevel::Defcon5;
}

}  // namespace csx::core
