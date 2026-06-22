#pragma once

#include "BufferedFrame.hpp"
#include "H264Encoder.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace csx::recording {

struct ClipWriteResult {
    bool success{false};
    double durationSeconds{0.0};
    std::filesystem::path outputPath;
    std::string format;
};

class Mp4Writer {
public:
    Mp4Writer() = default;

    [[nodiscard]] ClipWriteResult writeClip(const std::filesystem::path& outputPath,
                                            const std::vector<BufferedFrame>& frames,
                                            std::uint32_t fps) const;

private:
    [[nodiscard]] ClipWriteResult writeCsxClip(const std::filesystem::path& outputPath,
                                               const std::vector<BufferedFrame>& frames,
                                               std::uint32_t fps) const;
    [[nodiscard]] ClipWriteResult writeH264Clip(const std::filesystem::path& outputPath,
                                                const std::vector<BufferedFrame>& frames,
                                                std::uint32_t fps) const;
};

}  // namespace csx::recording
