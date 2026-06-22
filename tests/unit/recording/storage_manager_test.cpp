#include <gtest/gtest.h>

#include "StorageManager.hpp"

#include <filesystem>
#include <fstream>
#include <thread>

using csx::recording::RecordingSettings;
using csx::recording::StorageManager;

TEST(StorageManagerTest, RemovesOldestFilesWhenOverQuota) {
    const auto directory = std::filesystem::temp_directory_path() / "csx_storage_test";
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);

    const auto oldFile = directory / "old.bin";
    const auto newFile = directory / "new.bin";
    {
        std::ofstream old(oldFile, std::ios::binary);
        old << std::string(1024, 'a');
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    {
        std::ofstream newer(newFile, std::ios::binary);
        newer << std::string(1024, 'b');
    }

    RecordingSettings settings;
    settings.maxStorageGb = 0.000001;
    StorageManager manager(settings);

    const auto removed = manager.enforceQuota(directory);
    EXPECT_GE(removed, 1U);
    EXPECT_FALSE(std::filesystem::exists(oldFile));

    std::filesystem::remove_all(directory);
}
