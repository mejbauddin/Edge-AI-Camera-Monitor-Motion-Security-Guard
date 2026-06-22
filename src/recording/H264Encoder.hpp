#pragma once

#include "BufferedFrame.hpp"

#include <filesystem>
#include <string>

namespace csx::recording {

class H264Encoder {
public:
    H264Encoder() = default;

    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] bool open(const std::filesystem::path& outputPath, std::uint32_t width,
                            std::uint32_t height, std::uint32_t fps);
    [[nodiscard]] bool encodeFrame(const BufferedFrame& frame);
    [[nodiscard]] bool close(double& outDurationSeconds);

private:
#if defined(CSX_HAS_FFMPEG)
    bool openFFmpeg(const std::filesystem::path& outputPath, std::uint32_t width, std::uint32_t height,
                    std::uint32_t fps);
    bool encodeFrameFFmpeg(const BufferedFrame& frame);
    bool closeFFmpeg(double& outDurationSeconds);
    void* formatContext_{nullptr};
    void* codecContext_{nullptr};
    void* videoStream_{nullptr};
    void* frame_{nullptr};
    void* packet_{nullptr};
    std::uint64_t frameIndex_{0};
    std::uint32_t fps_{30};
#endif
    bool active_{false};
};

}  // namespace csx::recording
