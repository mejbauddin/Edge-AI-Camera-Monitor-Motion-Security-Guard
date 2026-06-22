#include "PathResolver.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace csx::utils {

namespace {

std::filesystem::path detectExecutableDirectory() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        return std::filesystem::current_path();
    }
    return std::filesystem::path(buffer).parent_path();
#else
    return std::filesystem::current_path();
#endif
}

}  // namespace

PathResolver::PathResolver() = default;

std::filesystem::path PathResolver::executableDirectory() const {
    return root();
}

std::filesystem::path PathResolver::root() const {
    if (!rootOverride_.empty()) {
        return rootOverride_;
    }
    return detectExecutableDirectory();
}

void PathResolver::setRootOverride(const std::filesystem::path& root) {
    rootOverride_ = root;
}

std::filesystem::path PathResolver::resolve(const std::string& relativePath) const {
    const std::filesystem::path candidate(relativePath);
    if (candidate.is_absolute()) {
        return candidate;
    }
    return root() / candidate;
}

std::filesystem::path PathResolver::configFile(const std::string& fileName) const {
    return resolve("config/" + fileName);
}

std::filesystem::path PathResolver::modelFile(const std::string& fileName) const {
    return resolve("assets/models/" + fileName);
}

std::filesystem::path PathResolver::databaseFile(const std::string& fileName) const {
    return resolve("data/" + fileName);
}

std::filesystem::path PathResolver::logDirectory() const {
    const auto directory = resolve("logs");
    std::filesystem::create_directories(directory);
    return directory;
}

std::filesystem::path PathResolver::recordingDirectory() const {
    const auto directory = resolve("recordings");
    std::filesystem::create_directories(directory);
    return directory;
}

std::filesystem::path PathResolver::assetDirectory(const std::string& subfolder) const {
    return resolve("assets/" + subfolder);
}

}  // namespace csx::utils
