#include "DatabaseConnection.hpp"

#include <sqlite3.h>

#include <filesystem>

namespace csx::database {

DatabaseConnection::DatabaseConnection() = default;

DatabaseConnection::~DatabaseConnection() {
    close();
}

bool DatabaseConnection::open(const std::string& path) {
    std::lock_guard lock(mutex_);
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }

    const auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    const int result = sqlite3_open_v2(path.c_str(), &db_,
                                       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                                       nullptr);
    return result == SQLITE_OK;
}

void DatabaseConnection::close() {
    std::lock_guard lock(mutex_);
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseConnection::isOpen() const noexcept {
    std::lock_guard lock(mutex_);
    return db_ != nullptr;
}

bool DatabaseConnection::execute(const std::string& sql) {
    std::lock_guard lock(mutex_);
    if (db_ == nullptr) {
        return false;
    }
    char* errorMessage = nullptr;
    const int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMessage);
    if (errorMessage != nullptr) {
        sqlite3_free(errorMessage);
    }
    return result == SQLITE_OK;
}

bool DatabaseConnection::prepareStatement(const std::string& sql, sqlite3_stmt** outStatement) {
    std::lock_guard lock(mutex_);
    if (db_ == nullptr || outStatement == nullptr) {
        return false;
    }
    return sqlite3_prepare_v2(db_, sql.c_str(), -1, outStatement, nullptr) == SQLITE_OK;
}

bool DatabaseConnection::step(sqlite3_stmt* statement) {
    if (statement == nullptr) {
        return false;
    }
    std::lock_guard lock(mutex_);
    const int result = sqlite3_step(statement);
    return result == SQLITE_ROW || result == SQLITE_DONE;
}

void DatabaseConnection::finalize(sqlite3_stmt* statement) {
    if (statement == nullptr) {
        return;
    }
    std::lock_guard lock(mutex_);
    sqlite3_finalize(statement);
}

std::int64_t DatabaseConnection::lastInsertRowId() const {
    std::lock_guard lock(mutex_);
    return db_ == nullptr ? 0 : sqlite3_last_insert_rowid(db_);
}

std::string DatabaseConnection::lastError() const {
    std::lock_guard lock(mutex_);
    return db_ == nullptr ? "Database not open" : sqlite3_errmsg(db_);
}

sqlite3* DatabaseConnection::handle() const noexcept {
    return db_;
}

std::mutex& DatabaseConnection::mutex() noexcept {
    return mutex_;
}

}  // namespace csx::database
