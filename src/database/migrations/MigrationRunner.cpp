#include "MigrationRunner.hpp"

#include <sqlite3.h>

namespace csx::database {

namespace {

constexpr const char* kMigrationV1 = R"SQL(
CREATE TABLE IF NOT EXISTS schema_version (
    version INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    role TEXT NOT NULL DEFAULT 'operator',
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS authorized_faces (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    embedding BLOB NOT NULL,
    thumbnail_path TEXT,
    created_at TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS unknown_faces (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    embedding BLOB NOT NULL,
    first_seen TEXT NOT NULL DEFAULT (datetime('now')),
    last_seen TEXT NOT NULL DEFAULT (datetime('now')),
    sighting_count INTEGER NOT NULL DEFAULT 1
);

CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type TEXT NOT NULL,
    camera_id TEXT,
    payload_json TEXT,
    timestamp TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS threats (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    level TEXT NOT NULL,
    score REAL NOT NULL,
    reason TEXT,
    track_ids_json TEXT,
    timestamp TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS recordings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT NOT NULL,
    duration_seconds REAL NOT NULL DEFAULT 0,
    trigger_level TEXT,
    camera_id TEXT,
    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS snapshots (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT NOT NULL,
    threat_id INTEGER,
    timestamp TEXT NOT NULL DEFAULT (datetime('now')),
    FOREIGN KEY(threat_id) REFERENCES threats(id) ON DELETE SET NULL
);

CREATE TABLE IF NOT EXISTS logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    level TEXT NOT NULL,
    module TEXT NOT NULL,
    message TEXT NOT NULL,
    timestamp TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY,
    value_json TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS analytics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    metric TEXT NOT NULL,
    value REAL NOT NULL,
    period TEXT,
    timestamp TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE INDEX IF NOT EXISTS idx_events_timestamp ON events(timestamp);
CREATE INDEX IF NOT EXISTS idx_threats_timestamp ON threats(timestamp);
CREATE INDEX IF NOT EXISTS idx_authorized_faces_user ON authorized_faces(user_id);
)SQL";

constexpr const char* kMigrationV2 = R"SQL(
ALTER TABLE users ADD COLUMN country TEXT NOT NULL DEFAULT '';
ALTER TABLE users ADD COLUMN clearance TEXT NOT NULL DEFAULT 'STANDARD';
ALTER TABLE users ADD COLUMN notes TEXT NOT NULL DEFAULT '';
)SQL";

}  // namespace

MigrationRunner::MigrationRunner(DatabaseConnection& connection) : connection_(connection) {}

bool MigrationRunner::ensureSchemaVersionTable() {
    return connection_.execute(
        "CREATE TABLE IF NOT EXISTS schema_version (version INTEGER NOT NULL);");
}

int MigrationRunner::currentVersion() const {
    sqlite3_stmt* statement = nullptr;
    if (!connection_.prepareStatement("SELECT version FROM schema_version LIMIT 1;", &statement)) {
        return 0;
    }

    int version = 0;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        version = sqlite3_column_int(statement, 0);
    }
    connection_.finalize(statement);
    return version;
}

bool MigrationRunner::applyMigration(const int version, const char* sql) {
    if (!connection_.execute(sql)) {
        return false;
    }
    return connection_.execute("DELETE FROM schema_version; INSERT INTO schema_version(version) VALUES (" +
                               std::to_string(version) + ");");
}

bool MigrationRunner::migrate() {
    if (!ensureSchemaVersionTable()) {
        return false;
    }

    const int version = currentVersion();
    if (version >= 2) {
        return true;
    }
    if (version < 1) {
        if (!applyMigration(1, kMigrationV1)) {
            return false;
        }
    }
    return applyMigration(2, kMigrationV2);
}

}  // namespace csx::database
