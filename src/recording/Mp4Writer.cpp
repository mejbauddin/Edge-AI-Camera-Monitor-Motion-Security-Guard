#include "Mp4Writer.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>

namespace csx::recording {

namespace {

constexpr char kCsxMagic[4] = {'C', 'S', 'X', '1'};

std::filesystem::path withExtension(const std::filesystem::path& path, const std::string& extension) {
    return path.parent_path() / (path.stem().string() + extension);
}

}  // namespace

ClipWriteResult Mp4Writer::writeH264Clip(const std::filesystem::path& outputPath,
                                         const std::vector<BufferedFrame>& frames,
                                         const std::uint32_t fps) const {
    ClipWriteResult result;
    if (frames.empty() || !frames.front().valid()) {
        return result;
    }

    H264Encoder encoder;
    if (!encoder.available()) {
        return result;
    }

    const auto& first = frames.front().frame;
    if (!encoder.open(outputPath, first.width, first.height, fps)) {
        return result;
    }

    for (const auto& buffered : frames) {
        if (!encoder.encodeFrame(buffered)) {
            double unusedDuration = 0.0;
            encoder.close(unusedDuration);
            return result;
        }
    }

    result.outputPath = outputPath;
    result.format = "mp4";
    result.success = encoder.close(result.durationSeconds);
    return result;
}

ClipWriteResult Mp4Writer::writeCsxClip(const std::filesystem::path& outputPath,
                                        const std::vector<BufferedFrame>& frames,
                                        const std::uint32_t fps) const {
    ClipWriteResult result;
    if (frames.empty() || !frames.front().valid()) {
        return result;
    }

    const auto clipPath = withExtension(outputPath, ".csxclip");
    std::ofstream output(clipPath, std::ios::binary);
    if (!output) {
        return result;
    }

    const auto& first = frames.front().frame;
    const std::uint32_t width = first.width;
    const std::uint32_t height = first.height;
    const std::uint32_t frameCount = static_cast<std::uint32_t>(frames.size());

    output.write(kCsxMagic, 4);
    output.write(reinterpret_cast<const char*>(&width), sizeof(width));
    output.write(reinterpret_cast<const char*>(&height), sizeof(height));
    output.write(reinterpret_cast<const char*>(&fps), sizeof(fps));
    output.write(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));

    for (const auto& buffered : frames) {
        const auto sequence = buffered.frame.sequence;
        const auto timestampNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                     buffered.timestamp.time_since_epoch())
                                     .count();
        const std::uint32_t dataSize =
            static_cast<std::uint32_t>(buffered.frame.bgrData ? buffered.frame.bgrData->size() : 0U);

        output.write(reinterpret_cast<const char*>(&sequence), sizeof(sequence));
        output.write(reinterpret_cast<const char*>(&timestampNs), sizeof(timestampNs));
        output.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
        if (dataSize > 0 && buffered.frame.bgrData != nullptr) {
            output.write(reinterpret_cast<const char*>(buffered.frame.bgrData->data()),
                         static_cast<std::streamsize>(dataSize));
        }
    }

    result.outputPath = clipPath;
    result.format = "csxclip";
    result.success = output.good();
    result.durationSeconds =
        fps > 0 ? static_cast<double>(frameCount) / static_cast<double>(fps) : 0.0;
    return result;
}

ClipWriteResult Mp4Writer::writeClip(const std::filesystem::path& outputPath,
                                     const std::vector<BufferedFrame>& frames,
                                     const std::uint32_t fps) const {
    if (H264Encoder encoder; encoder.available()) {
        const auto h264Result = writeH264Clip(outputPath, frames, fps);
        if (h264Result.success) {
            return h264Result;
        }
    }
    return writeCsxClip(outputPath, frames, fps);
}

}  // namespace csx::recording
