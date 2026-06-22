#include "StorageManager.hpp"

#include <algorithm>
#include <vector>

namespace csx::recording {

StorageManager::StorageManager(RecordingSettings settings) : settings_(std::move(settings)) {}

void StorageManager::setSettings(const RecordingSettings& settings) {
    settings_ = settings;
}

StorageStats StorageManager::scan(const std::filesystem::path& directory) const {
    StorageStats stats;
    if (!std::filesystem::exists(directory)) {
        return stats;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        stats.usedGb += static_cast<double>(entry.file_size()) / (1024.0 * 1024.0 * 1024.0);
        ++stats.fileCount;
    }
    return stats;
}

std::size_t StorageManager::enforceQuota(const std::filesystem::path& directory) const {
    if (!std::filesystem::exists(directory) || settings_.maxStorageGb <= 0.0) {
        return 0;
    }

    struct FileEntry {
        std::filesystem::path path;
        std::filesystem::file_time_type modified;
        std::uintmax_t size{0};
    };

    std::vector<FileEntry> files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        files.push_back({entry.path(), entry.last_write_time(), entry.file_size()});
    }

    double usedGb = 0.0;
    for (const auto& file : files) {
        usedGb += static_cast<double>(file.size) / (1024.0 * 1024.0 * 1024.0);
    }

    if (usedGb <= settings_.maxStorageGb) {
        return 0;
    }

    std::sort(files.begin(), files.end(), [](const FileEntry& lhs, const FileEntry& rhs) {
        return lhs.modified < rhs.modified;
    });

    std::size_t removed = 0;
    for (const auto& file : files) {
        if (usedGb <= settings_.maxStorageGb) {
            break;
        }
        std::error_code error;
        std::filesystem::remove(file.path, error);
        if (!error) {
            usedGb -= static_cast<double>(file.size) / (1024.0 * 1024.0 * 1024.0);
            ++removed;
        }
    }
    return removed;
}

}  // namespace csx::recording
