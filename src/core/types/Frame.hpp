#pragma once

#include "types/Enums.hpp"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace csx::core {

struct CameraDiagnosticsSnapshot {
    float fps{0.0F};
    float latencyMs{0.0F};
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::uint64_t droppedFrames{0};
    CameraState state{CameraState::Disconnected};
};

struct Frame {
    std::uint64_t sequence{0};
    std::string cameraId;
    std::chrono::steady_clock::time_point captureTime{};
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::shared_ptr<const std::vector<std::uint8_t>> bgrData;
    CameraDiagnosticsSnapshot diagnostics;

    [[nodiscard]] bool valid() const noexcept {
        return bgrData != nullptr && !bgrData->empty() && width > 0 && height > 0;
    }
};

struct Point2f {
    float x{0.0F};
    float y{0.0F};
};

struct Rect2f {
    float x{0.0F};
    float y{0.0F};
    float width{0.0F};
    float height{0.0F};

    [[nodiscard]] Point2f center() const noexcept {
        return {x + width * 0.5F, y + height * 0.5F};
    }
};

struct Track {
    std::uint32_t id{0};
    Rect2f bbox{};
    Point2f center{};
    Point2f velocity{};
    Point2f acceleration{};
    Point2f predictedPosition{};
    float directionDegrees{0.0F};
    float confidence{0.0F};
    std::uint32_t ageFrames{0};
    bool active{false};
    std::vector<Point2f> history;
};

struct FaceMatch {
    std::uint32_t trackId{0};
    Rect2f bbox{};
    std::string identityName;
    std::string country;
    std::string role;
    std::string clearance;
    IdentityClassification classification{IdentityClassification::Unknown};
    float confidence{0.0F};
    float embeddingDistance{1.0F};
    std::string userId;
};

struct BehaviorAnomaly {
    BehaviorAnomalyType type{BehaviorAnomalyType::None};
    float score{0.0F};
    Rect2f region{};
    std::chrono::milliseconds duration{0};
    std::string description;
    std::uint32_t trackId{0};
};

struct ThreatAssessment {
    ThreatLevel level{ThreatLevel::Green};
    DefconLevel defcon{DefconLevel::Defcon5};
    float threatScore{0.0F};
    float behaviorScore{0.0F};
    float confidence{0.0F};
    int priority{0};
    int severity{0};
    std::string decisionReason;
    std::vector<std::uint32_t> contributingTrackIds;
    std::chrono::system_clock::time_point timestamp{};
};

struct EngineHealth {
    EngineStatus status{EngineStatus::Offline};
    float confidence{0.0F};
    float lastLatencyMs{0.0F};
    std::uint64_t processedFrames{0};
    std::string detail;
};

struct SystemHealth {
    float cpuPercent{0.0F};
    float ramPercent{0.0F};
    float gpuPercent{0.0F};
    float temperatureC{0.0F};
    float frameTimeMs{0.0F};
    float fps{0.0F};
    EngineHealth camera;
    EngineHealth motion;
    EngineHealth tracking;
    EngineHealth recognition;
    EngineHealth behavior;
    EngineHealth threat;
    EngineHealth recorder;
    EngineHealth database;
    EngineHealth voice;
};

}  // namespace csx::core
