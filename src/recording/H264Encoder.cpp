#include "H264Encoder.hpp"

namespace csx::recording {

bool H264Encoder::available() const noexcept {
#if defined(CSX_HAS_FFMPEG)
    return true;
#else
    return false;
#endif
}

bool H264Encoder::open(const std::filesystem::path& outputPath, const std::uint32_t width,
                       const std::uint32_t height, const std::uint32_t fps) {
#if defined(CSX_HAS_FFMPEG)
    return openFFmpeg(outputPath, width, height, fps);
#else
    (void)outputPath;
    (void)width;
    (void)height;
    (void)fps;
    return false;
#endif
}

bool H264Encoder::encodeFrame(const BufferedFrame& frame) {
#if defined(CSX_HAS_FFMPEG)
    return encodeFrameFFmpeg(frame);
#else
    (void)frame;
    return false;
#endif
}

bool H264Encoder::close(double& outDurationSeconds) {
#if defined(CSX_HAS_FFMPEG)
    return closeFFmpeg(outDurationSeconds);
#else
    outDurationSeconds = 0.0;
    return false;
#endif
}

#if defined(CSX_HAS_FFMPEG)
// FFmpeg integration point — populated when libavcodec is linked at build time.
bool H264Encoder::openFFmpeg(const std::filesystem::path&, std::uint32_t, std::uint32_t, std::uint32_t) {
    return false;
}
bool H264Encoder::encodeFrameFFmpeg(const BufferedFrame&) { return false; }
bool H264Encoder::closeFFmpeg(double& outDurationSeconds) {
    outDurationSeconds = 0.0;
    return false;
}
#endif

}  // namespace csx::recording
