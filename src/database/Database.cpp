#include "Database.hpp"

namespace csx::database {

Database::Database()
    : migrationRunner_(connection_),
      users_(connection_),
      faces_(connection_),
      events_(connection_),
      threats_(connection_),
      recordings_(connection_),
      settings_(connection_) {}

bool Database::open(const std::string& path) {
    if (!connection_.open(path)) {
        health_.status = core::EngineStatus::Fault;
        health_.detail = connection_.lastError();
        return false;
    }

    if (!migrationRunner_.migrate()) {
        health_.status = core::EngineStatus::Fault;
        health_.detail = "Migration failed: " + connection_.lastError();
        connection_.close();
        return false;
    }

    health_.status = core::EngineStatus::Online;
    health_.detail = "Database online";
    health_.confidence = 1.0F;
    return true;
}

void Database::close() {
    connection_.close();
    health_.status = core::EngineStatus::Offline;
    health_.detail = "Database offline";
}

bool Database::isOpen() const {
    return connection_.isOpen();
}

core::EngineHealth Database::health() const {
    return health_;
}

UserRepository& Database::users() { return users_; }
FaceRepository& Database::faces() { return faces_; }
EventRepository& Database::events() { return events_; }
ThreatRepository& Database::threats() { return threats_; }
RecordingRepository& Database::recordings() { return recordings_; }
SettingsRepository& Database::settings() { return settings_; }

const UserRepository& Database::users() const { return users_; }
const FaceRepository& Database::faces() const { return faces_; }

std::shared_ptr<Database> createDatabase() {
    return std::make_shared<Database>();
}

}  // namespace csx::database
