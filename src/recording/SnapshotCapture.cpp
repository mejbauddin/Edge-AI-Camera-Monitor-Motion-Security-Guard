#include "SnapshotCapture.hpp"

#include <fstream>

namespace csx::recording {

namespace {

std::filesystem::path withExtension(const std::filesystem::path& path, const std::string& extension) {
    return path.parent_path() / (path.stem().string() + extension);
}

}  // namespace

std::optional<std::filesystem::path> SnapshotCapture::capturePpm(const core::Frame& frame,
                                                                 const std::filesystem::path& outputPath) const {
    if (!frame.valid() || frame.bgrData == nullptr) {
        return std::nullopt;
    }

    const auto ppmPath = withExtension(outputPath, ".ppm");
    std::ofstream output(ppmPath, std::ios::binary);
    if (!output) {
        return std::nullopt;
    }

    output << "P6\n" << frame.width << " " << frame.height << "\n255\n";
    for (std::uint32_t y = 0; y < frame.height; ++y) {
        for (std::uint32_t x = 0; x < frame.width; ++x) {
            const std::size_t index =
                (static_cast<std::size_t>(y) * frame.width + static_cast<std::size_t>(x)) * 3U;
            const auto& pixels = *frame.bgrData;
            output.put(static_cast<char>(pixels[index + 2]));
            output.put(static_cast<char>(pixels[index + 1]));
            output.put(static_cast<char>(pixels[index]));
        }
    }

    if (!output.good()) {
        return std::nullopt;
    }
    return ppmPath;
}

std::optional<std::filesystem::path> SnapshotCapture::captureBgr(const core::Frame& frame,
                                                                 const std::filesystem::path& outputPath) const {
    if (!frame.valid() || frame.bgrData == nullptr) {
        return std::nullopt;
    }

    const auto rawPath = withExtension(outputPath, ".bgr");
    std::ofstream output(rawPath, std::ios::binary);
    if (!output) {
        return std::nullopt;
    }

    const std::uint32_t header[3] = {frame.width, frame.height, 3U};
    output.write(reinterpret_cast<const char*>(header), sizeof(header));
    output.write(reinterpret_cast<const char*>(frame.bgrData->data()),
                 static_cast<std::streamsize>(frame.bgrData->size()));

    if (!output.good()) {
        return std::nullopt;
    }
    return rawPath;
}

}  // namespace csx::recording
