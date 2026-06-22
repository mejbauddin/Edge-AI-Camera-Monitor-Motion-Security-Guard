#pragma once

namespace csx::core::events {

inline constexpr const char* kFrameCaptured     = "frame.captured";
inline constexpr const char* kTracksUpdated     = "tracks.updated";
inline constexpr const char* kFaceRecognized    = "face.recognized";
inline constexpr const char* kBehaviorAnomaly   = "behavior.anomaly";
inline constexpr const char* kThreatElevated    = "threat.elevated";
inline constexpr const char* kRecordingStarted  = "recording.started";
inline constexpr const char* kRecordingStopped  = "recording.stopped";
inline constexpr const char* kConfigReloaded    = "config.reloaded";
inline constexpr const char* kSystemHealth      = "system.health";
inline constexpr const char* kCameraState       = "camera.state";
inline constexpr const char* kDefconChanged     = "defcon.changed";
inline constexpr const char* kBootPhase         = "boot.phase";
inline constexpr const char* kAlertTriggered    = "alert.triggered";

}  // namespace csx::core::events
