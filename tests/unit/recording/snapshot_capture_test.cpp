#include <gtest/gtest.h>

#include "SnapshotCapture.hpp"

#include <filesystem>
#include <fstream>

using csx::recording::SnapshotCapture;

namespace {

csx::core::Frame makeFrame() {
    csx::core::Frame frame;
    frame.sequence = 1;
    frame.width = 4;
    frame.height = 2;
    frame.bgrData = std::make_shared<const std::vector<std::uint8_t>>(
        std::vector<std::uint8_t>{255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 0, 128,
                                  128, 128, 255, 255, 255});
    return frame;
}

}  // namespace

TEST(SnapshotCaptureTest, WritesPpmSnapshot) {
    SnapshotCapture capture;
    const auto output =
        std::filesystem::temp_directory_path() / "csx_snapshot_test_unique";

    const auto saved = capture.capturePpm(makeFrame(), output);
    ASSERT_TRUE(saved.has_value());
    EXPECT_TRUE(std::filesystem::exists(*saved));
    EXPECT_EQ(saved->extension().string(), ".ppm");

    {
        std::ifstream file(*saved, std::ios::binary);
        std::string magic;
        file >> magic;
        EXPECT_EQ(magic, "P6");
    }

    std::error_code error;
    std::filesystem::remove(*saved, error);
    EXPECT_FALSE(error) << error.message();
}
