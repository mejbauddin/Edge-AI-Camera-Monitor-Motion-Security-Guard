#include <gtest/gtest.h>

#include "PathResolver.hpp"

#include <filesystem>

using csx::utils::PathResolver;

TEST(PathResolverTest, ResolvesRelativePathsFromOverrideRoot) {
    PathResolver resolver;
    resolver.setRootOverride("D:/CyberSentinelX-root");

    EXPECT_EQ(resolver.configFile("default.json").generic_string(), "D:/CyberSentinelX-root/config/default.json");
    EXPECT_EQ(resolver.modelFile("yunet.onnx").generic_string(),
              "D:/CyberSentinelX-root/assets/models/yunet.onnx");
    EXPECT_EQ(resolver.databaseFile().generic_string(), "D:/CyberSentinelX-root/data/cyber_sentinel.db");
}

TEST(PathResolverTest, KeepsAbsolutePaths) {
    PathResolver resolver;
    resolver.setRootOverride("D:/root");

    const auto absolute = resolver.resolve("C:/absolute/path/file.json");
    EXPECT_EQ(absolute.generic_string(), "C:/absolute/path/file.json");
}

TEST(PathResolverTest, CreatesLogAndRecordingDirectories) {
    PathResolver resolver;
    const auto tempRoot = std::filesystem::temp_directory_path() / "csx_path_resolver_test";
    std::filesystem::remove_all(tempRoot);
    resolver.setRootOverride(tempRoot);

    const auto logs = resolver.logDirectory();
    const auto recordings = resolver.recordingDirectory();

    EXPECT_TRUE(std::filesystem::exists(logs));
    EXPECT_TRUE(std::filesystem::exists(recordings));

    std::filesystem::remove_all(tempRoot);
}
