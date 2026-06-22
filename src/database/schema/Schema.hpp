#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace csx::database {

struct UserRecord {
    std::int64_t id{0};
    std::string name;
    std::string role;
    std::string country;
    std::string clearance;
    std::string notes;
    std::string createdAt;
};

struct AuthorizedFaceRecord {
    std::int64_t id{0};
    std::int64_t userId{0};
    std::string userName;
    std::string role;
    std::string country;
    std::string clearance;
    std::vector<float> embedding;
    std::string thumbnailPath;
    std::string createdAt;
};

struct UnknownFaceRecord {
    std::int64_t id{0};
    std::vector<float> embedding;
    std::string firstSeen;
    std::string lastSeen;
    std::int64_t sightingCount{0};
};

struct EventRecord {
    std::int64_t id{0};
    std::string type;
    std::string cameraId;
    std::string payloadJson;
    std::string timestamp;
};

struct ThreatRecord {
    std::int64_t id{0};
    std::string level;
    float score{0.0F};
    std::string reason;
    std::string trackIdsJson;
    std::string timestamp;
};

struct RecordingRecord {
    std::int64_t id{0};
    std::string path;
    double durationSeconds{0.0};
    std::string triggerLevel;
    std::string cameraId;
    std::string createdAt;
};

struct SettingRecord {
    std::string key;
    std::string valueJson;
};

}  // namespace csx::database
