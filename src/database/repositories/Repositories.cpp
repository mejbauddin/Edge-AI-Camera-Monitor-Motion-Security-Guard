#include "repositories/Repositories.hpp"

#include <sqlite3.h>

#include <cstring>

namespace csx::database {

namespace {

std::string columnText(sqlite3_stmt* statement, int index) {
    const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(statement, index));
    return text == nullptr ? "" : text;
}

}  // namespace

UserRepository::UserRepository(DatabaseConnection& connection) : connection_(connection) {}

std::optional<std::int64_t> UserRepository::createUser(const std::string& name, const std::string& role,
                                                       const std::string& country,
                                                       const std::string& clearance,
                                                       const std::string& notes) {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "INSERT INTO users(name, role, country, clearance, notes) VALUES (?, ?, ?, ?, ?);",
            &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, country.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, clearance.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 5, notes.c_str(), -1, SQLITE_TRANSIENT);
    const bool ok = connection_.step(statement);
    connection_.finalize(statement);
    return ok ? std::optional<std::int64_t>{connection_.lastInsertRowId()} : std::nullopt;
}

std::optional<UserRecord> UserRepository::findById(const std::int64_t id) const {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "SELECT id, name, role, COALESCE(country,''), COALESCE(clearance,'STANDARD'), COALESCE(notes,''), created_at "
            "FROM users WHERE id = ?;",
            &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_int64(statement, 1, id);
    std::optional<UserRecord> record;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        record = UserRecord{sqlite3_column_int64(statement, 0), columnText(statement, 1),
                            columnText(statement, 2), columnText(statement, 3),
                            columnText(statement, 4), columnText(statement, 5),
                            columnText(statement, 6)};
    }
    connection_.finalize(statement);
    return record;
}

std::vector<UserRecord> UserRepository::listAll() const {
    std::vector<UserRecord> users;
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "SELECT id, name, role, COALESCE(country,''), COALESCE(clearance,'STANDARD'), COALESCE(notes,''), created_at "
            "FROM users ORDER BY id;",
            &statement)) {
        return users;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        users.push_back(UserRecord{sqlite3_column_int64(statement, 0), columnText(statement, 1),
                                   columnText(statement, 2), columnText(statement, 3),
                                   columnText(statement, 4), columnText(statement, 5),
                                   columnText(statement, 6)});
    }
    connection_.finalize(statement);
    return users;
}

std::vector<float> FaceRepository::blobToEmbedding(const void* data, const int bytes) {
    std::vector<float> embedding;
    if (data == nullptr || bytes <= 0) {
        return embedding;
    }
    const auto floatCount = static_cast<std::size_t>(bytes) / sizeof(float);
    embedding.resize(floatCount);
    std::memcpy(embedding.data(), data, floatCount * sizeof(float));
    return embedding;
}

std::vector<std::uint8_t> FaceRepository::embeddingToBlob(const std::vector<float>& embedding) {
    std::vector<std::uint8_t> blob(embedding.size() * sizeof(float));
    if (!embedding.empty()) {
        std::memcpy(blob.data(), embedding.data(), blob.size());
    }
    return blob;
}

FaceRepository::FaceRepository(DatabaseConnection& connection) : connection_(connection) {}

std::optional<std::int64_t> FaceRepository::addAuthorizedFace(const std::int64_t userId,
                                                              const std::vector<float>& embedding,
                                                              const std::string& thumbnailPath) {
    const auto blob = embeddingToBlob(embedding);
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "INSERT INTO authorized_faces(user_id, embedding, thumbnail_path) VALUES (?, ?, ?);",
            &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_int64(statement, 1, userId);
    sqlite3_bind_blob(statement, 2, blob.data(), static_cast<int>(blob.size()), SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, thumbnailPath.c_str(), -1, SQLITE_TRANSIENT);
    const bool ok = connection_.step(statement);
    connection_.finalize(statement);
    return ok ? std::optional<std::int64_t>{connection_.lastInsertRowId()} : std::nullopt;
}

std::vector<AuthorizedFaceRecord> FaceRepository::listAuthorizedFaces() const {
    std::vector<AuthorizedFaceRecord> faces;
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "SELECT af.id, af.user_id, u.name, u.role, COALESCE(u.country,''), COALESCE(u.clearance,'STANDARD'), "
            "af.embedding, af.thumbnail_path, af.created_at "
            "FROM authorized_faces af JOIN users u ON u.id = af.user_id ORDER BY af.id;",
            &statement)) {
        return faces;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        AuthorizedFaceRecord record;
        record.id = sqlite3_column_int64(statement, 0);
        record.userId = sqlite3_column_int64(statement, 1);
        record.userName = columnText(statement, 2);
        record.role = columnText(statement, 3);
        record.country = columnText(statement, 4);
        record.clearance = columnText(statement, 5);
        record.embedding =
            blobToEmbedding(sqlite3_column_blob(statement, 6), sqlite3_column_bytes(statement, 6));
        record.thumbnailPath = columnText(statement, 7);
        record.createdAt = columnText(statement, 8);
        faces.push_back(std::move(record));
    }
    connection_.finalize(statement);
    return faces;
}

std::optional<std::int64_t> FaceRepository::upsertUnknownFace(const std::vector<float>& embedding) {
    const auto blob = embeddingToBlob(embedding);
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "INSERT INTO unknown_faces(embedding) VALUES (?);", &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_blob(statement, 1, blob.data(), static_cast<int>(blob.size()), SQLITE_TRANSIENT);
    const bool ok = connection_.step(statement);
    connection_.finalize(statement);
    return ok ? std::optional<std::int64_t>{connection_.lastInsertRowId()} : std::nullopt;
}

std::vector<UnknownFaceRecord> FaceRepository::listUnknownFaces() const {
    std::vector<UnknownFaceRecord> faces;
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "SELECT id, embedding, first_seen, last_seen, sighting_count FROM unknown_faces ORDER BY id;",
            &statement)) {
        return faces;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        UnknownFaceRecord record;
        record.id = sqlite3_column_int64(statement, 0);
        record.embedding =
            blobToEmbedding(sqlite3_column_blob(statement, 1), sqlite3_column_bytes(statement, 1));
        record.firstSeen = columnText(statement, 2);
        record.lastSeen = columnText(statement, 3);
        record.sightingCount = sqlite3_column_int64(statement, 4);
        faces.push_back(std::move(record));
    }
    connection_.finalize(statement);
    return faces;
}

EventRepository::EventRepository(DatabaseConnection& connection) : connection_(connection) {}

std::optional<std::int64_t> EventRepository::insertEvent(const std::string& type,
                                                         const std::string& cameraId,
                                                         const std::string& payloadJson) {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "INSERT INTO events(type, camera_id, payload_json) VALUES (?, ?, ?);", &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, cameraId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, payloadJson.c_str(), -1, SQLITE_TRANSIENT);
    const bool ok = connection_.step(statement);
    connection_.finalize(statement);
    return ok ? std::optional<std::int64_t>{connection_.lastInsertRowId()} : std::nullopt;
}

std::vector<EventRecord> EventRepository::recentEvents(const std::size_t limit) const {
    std::vector<EventRecord> events;
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "SELECT id, type, camera_id, payload_json, timestamp FROM events ORDER BY id DESC LIMIT ?;",
            &statement)) {
        return events;
    }

    sqlite3_bind_int64(statement, 1, static_cast<std::int64_t>(limit));
    while (sqlite3_step(statement) == SQLITE_ROW) {
        events.push_back(EventRecord{sqlite3_column_int64(statement, 0), columnText(statement, 1),
                                     columnText(statement, 2), columnText(statement, 3),
                                     columnText(statement, 4)});
    }
    connection_.finalize(statement);
    return events;
}

ThreatRepository::ThreatRepository(DatabaseConnection& connection) : connection_(connection) {}

std::optional<std::int64_t> ThreatRepository::insertThreat(const std::string& level, const float score,
                                                            const std::string& reason,
                                                            const std::string& trackIdsJson) {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "INSERT INTO threats(level, score, reason, track_ids_json) VALUES (?, ?, ?, ?);",
            &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, level.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(statement, 2, score);
    sqlite3_bind_text(statement, 3, reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, trackIdsJson.c_str(), -1, SQLITE_TRANSIENT);
    const bool ok = connection_.step(statement);
    connection_.finalize(statement);
    return ok ? std::optional<std::int64_t>{connection_.lastInsertRowId()} : std::nullopt;
}

std::vector<ThreatRecord> ThreatRepository::recentThreats(const std::size_t limit) const {
    std::vector<ThreatRecord> threats;
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "SELECT id, level, score, reason, track_ids_json, timestamp FROM threats ORDER BY id DESC LIMIT ?;",
            &statement)) {
        return threats;
    }

    sqlite3_bind_int64(statement, 1, static_cast<std::int64_t>(limit));
    while (sqlite3_step(statement) == SQLITE_ROW) {
        threats.push_back(ThreatRecord{sqlite3_column_int64(statement, 0), columnText(statement, 1),
                                       static_cast<float>(sqlite3_column_double(statement, 2)),
                                       columnText(statement, 3), columnText(statement, 4),
                                       columnText(statement, 5)});
    }
    connection_.finalize(statement);
    return threats;
}

RecordingRepository::RecordingRepository(DatabaseConnection& connection) : connection_(connection) {}

std::optional<std::int64_t> RecordingRepository::insertRecording(const std::string& path,
                                                                 const double durationSeconds,
                                                                 const std::string& triggerLevel,
                                                                 const std::string& cameraId) {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "INSERT INTO recordings(path, duration_seconds, trigger_level, camera_id) VALUES (?, ?, ?, ?);",
            &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, path.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(statement, 2, durationSeconds);
    sqlite3_bind_text(statement, 3, triggerLevel.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, cameraId.c_str(), -1, SQLITE_TRANSIENT);
    const bool ok = connection_.step(statement);
    connection_.finalize(statement);
    return ok ? std::optional<std::int64_t>{connection_.lastInsertRowId()} : std::nullopt;
}

std::vector<RecordingRecord> RecordingRepository::listRecordings(const std::size_t limit) const {
    std::vector<RecordingRecord> recordings;
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "SELECT id, path, duration_seconds, trigger_level, camera_id, created_at FROM recordings ORDER BY id DESC LIMIT ?;",
            &statement)) {
        return recordings;
    }

    sqlite3_bind_int64(statement, 1, static_cast<std::int64_t>(limit));
    while (sqlite3_step(statement) == SQLITE_ROW) {
        recordings.push_back(
            RecordingRecord{sqlite3_column_int64(statement, 0), columnText(statement, 1),
                            sqlite3_column_double(statement, 2), columnText(statement, 3),
                            columnText(statement, 4), columnText(statement, 5)});
    }
    connection_.finalize(statement);
    return recordings;
}

SettingsRepository::SettingsRepository(DatabaseConnection& connection) : connection_(connection) {}

bool SettingsRepository::upsertSetting(const std::string& key, const std::string& valueJson) {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement(
            "INSERT INTO settings(key, value_json) VALUES (?, ?) "
            "ON CONFLICT(key) DO UPDATE SET value_json = excluded.value_json;",
            &statement)) {
        return false;
    }

    sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, valueJson.c_str(), -1, SQLITE_TRANSIENT);
    const bool ok = connection_.step(statement);
    connection_.finalize(statement);
    return ok;
}

std::optional<SettingRecord> SettingsRepository::getSetting(const std::string& key) const {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement("SELECT key, value_json FROM settings WHERE key = ?;",
                                      &statement)) {
        return std::nullopt;
    }

    sqlite3_bind_text(statement, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<SettingRecord> record;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        record = SettingRecord{columnText(statement, 0), columnText(statement, 1)};
    }
    connection_.finalize(statement);
    return record;
}

}  // namespace csx::database
