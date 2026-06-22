#pragma once

#include "DatabaseConnection.hpp"
#include "interfaces/Interfaces.hpp"
#include "migrations/MigrationRunner.hpp"
#include "repositories/Repositories.hpp"

#include <memory>
#include <string>

namespace csx::database {

class Database final : public core::IDatabase {
public:
    Database();

    bool open(const std::string& path) override;
    void close() override;
    [[nodiscard]] bool isOpen() const override;
    [[nodiscard]] core::EngineHealth health() const override;

    [[nodiscard]] UserRepository& users();
    [[nodiscard]] FaceRepository& faces();
    [[nodiscard]] EventRepository& events();
    [[nodiscard]] ThreatRepository& threats();
    [[nodiscard]] RecordingRepository& recordings();
    [[nodiscard]] SettingsRepository& settings();

    [[nodiscard]] const UserRepository& users() const;
    [[nodiscard]] const FaceRepository& faces() const;

private:
    DatabaseConnection connection_;
    MigrationRunner migrationRunner_;
    UserRepository users_;
    FaceRepository faces_;
    EventRepository events_;
    ThreatRepository threats_;
    RecordingRepository recordings_;
    SettingsRepository settings_;
    core::EngineHealth health_;
};

std::shared_ptr<Database> createDatabase();

}  // namespace csx::database
