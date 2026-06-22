#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

namespace csx::database {

class DatabaseConnection {
public:
    DatabaseConnection();
    ~DatabaseConnection();

    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    bool open(const std::string& path);
    void close();
    [[nodiscard]] bool isOpen() const noexcept;

    bool execute(const std::string& sql);
    bool prepareStatement(const std::string& sql, sqlite3_stmt** outStatement);
    bool step(sqlite3_stmt* statement);
    void finalize(sqlite3_stmt* statement);

    [[nodiscard]] std::int64_t lastInsertRowId() const;
    [[nodiscard]] std::string lastError() const;

    [[nodiscard]] sqlite3* handle() const noexcept;
    [[nodiscard]] std::mutex& mutex() noexcept;

private:
    sqlite3* db_{nullptr};
    mutable std::mutex mutex_;
};

}  // namespace csx::database
