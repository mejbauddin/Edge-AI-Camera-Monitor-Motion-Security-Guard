#pragma once

#include "RecordingSettings.hpp"

#include <filesystem>

namespace csx::recording {

struct StorageStats {
    double usedGb{0.0};
    std::size_t fileCount{0};
};

class StorageManager {
public:
    explicit StorageManager(RecordingSettings settings = {});

    [[nodiscard]] StorageStats scan(const std::filesystem::path& directory) const;
    [[nodiscard]] std::size_t enforceQuota(const std::filesystem::path& directory) const;
    void setSettings(const RecordingSettings& settings);

private:
    RecordingSettings settings_;
};

}  // namespace csx::recording
