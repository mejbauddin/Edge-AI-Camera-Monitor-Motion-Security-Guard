#pragma once

#include "types/Frame.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace csx::recording {

class SnapshotCapture {
public:
    [[nodiscard]] std::optional<std::filesystem::path> captureBgr(const core::Frame& frame,
                                                                  const std::filesystem::path& outputPath) const;
    [[nodiscard]] std::optional<std::filesystem::path> capturePpm(const core::Frame& frame,
                                                                  const std::filesystem::path& outputPath) const;
};

}  // namespace csx::recording
