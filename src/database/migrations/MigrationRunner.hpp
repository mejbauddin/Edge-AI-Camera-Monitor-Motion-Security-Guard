#pragma once

#include "DatabaseConnection.hpp"

namespace csx::database {

class MigrationRunner {
public:
    explicit MigrationRunner(DatabaseConnection& connection);

    bool migrate();

private:
    bool ensureSchemaVersionTable();
    [[nodiscard]] int currentVersion() const;
    bool applyMigration(int version, const char* sql);

    DatabaseConnection& connection_;
};

}  // namespace csx::database
