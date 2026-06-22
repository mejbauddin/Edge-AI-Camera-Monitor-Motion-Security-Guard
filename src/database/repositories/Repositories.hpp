#pragma once

#include "DatabaseConnection.hpp"
#include "schema/Schema.hpp"

#include <optional>
#include <string>
#include <vector>

namespace csx::database {

class UserRepository {
public:
    explicit UserRepository(DatabaseConnection& connection);

    std::optional<std::int64_t> createUser(const std::string& name, const std::string& role = "operator",
                                           const std::string& country = "",
                                           const std::string& clearance = "STANDARD",
                                           const std::string& notes = "");
    std::optional<UserRecord> findById(std::int64_t id) const;
    std::vector<UserRecord> listAll() const;

private:
    DatabaseConnection& connection_;
};

class FaceRepository {
public:
    explicit FaceRepository(DatabaseConnection& connection);

    std::optional<std::int64_t> addAuthorizedFace(std::int64_t userId, const std::vector<float>& embedding,
                                                  const std::string& thumbnailPath = "");
    std::vector<AuthorizedFaceRecord> listAuthorizedFaces() const;

    std::optional<std::int64_t> upsertUnknownFace(const std::vector<float>& embedding);
    std::vector<UnknownFaceRecord> listUnknownFaces() const;

    static std::vector<float> blobToEmbedding(const void* data, int bytes);
    static std::vector<std::uint8_t> embeddingToBlob(const std::vector<float>& embedding);

private:
    DatabaseConnection& connection_;
};

class EventRepository {
public:
    explicit EventRepository(DatabaseConnection& connection);

    std::optional<std::int64_t> insertEvent(const std::string& type, const std::string& cameraId,
                                            const std::string& payloadJson);
    std::vector<EventRecord> recentEvents(std::size_t limit = 100) const;

private:
    DatabaseConnection& connection_;
};

class ThreatRepository {
public:
    explicit ThreatRepository(DatabaseConnection& connection);

    std::optional<std::int64_t> insertThreat(const std::string& level, float score, const std::string& reason,
                                             const std::string& trackIdsJson);
    std::vector<ThreatRecord> recentThreats(std::size_t limit = 100) const;

private:
    DatabaseConnection& connection_;
};

class RecordingRepository {
public:
    explicit RecordingRepository(DatabaseConnection& connection);

    std::optional<std::int64_t> insertRecording(const std::string& path, double durationSeconds,
                                                  const std::string& triggerLevel,
                                                  const std::string& cameraId);
    std::vector<RecordingRecord> listRecordings(std::size_t limit = 100) const;

private:
    DatabaseConnection& connection_;
};

class SettingsRepository {
public:
    explicit SettingsRepository(DatabaseConnection& connection);

    bool upsertSetting(const std::string& key, const std::string& valueJson);
    std::optional<SettingRecord> getSetting(const std::string& key) const;

private:
    DatabaseConnection& connection_;
};

}  // namespace csx::database
