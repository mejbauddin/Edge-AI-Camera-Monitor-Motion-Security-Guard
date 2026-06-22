#pragma once

#include <filesystem>
#include <string>

namespace csx::utils {

class PathResolver {
public:
    PathResolver();

    [[nodiscard]] std::filesystem::path executableDirectory() const;
    [[nodiscard]] std::filesystem::path resolve(const std::string& relativePath) const;
    [[nodiscard]] std::filesystem::path configFile(const std::string& fileName = "default.json") const;
    [[nodiscard]] std::filesystem::path modelFile(const std::string& fileName) const;
    [[nodiscard]] std::filesystem::path databaseFile(const std::string& fileName = "cyber_sentinel.db") const;
    [[nodiscard]] std::filesystem::path logDirectory() const;
    [[nodiscard]] std::filesystem::path recordingDirectory() const;
    [[nodiscard]] std::filesystem::path assetDirectory(const std::string& subfolder) const;

    void setRootOverride(const std::filesystem::path& root);

private:
    [[nodiscard]] std::filesystem::path root() const;

    std::filesystem::path rootOverride_;
};

}  // namespace csx::utils
