#include <gtest/gtest.h>

#include "SyntheticCameraSource.hpp"

using csx::camera::CameraSettings;
using csx::camera::SyntheticCameraSource;

TEST(SyntheticCameraSourceTest, GeneratesValidFrames) {
    CameraSettings settings;
    settings.width = 64;
    settings.height = 48;

    SyntheticCameraSource camera(settings);
    ASSERT_TRUE(camera.open("synthetic:0"));
    EXPECT_TRUE(camera.isOpen());

    csx::core::Frame frame;
    ASSERT_TRUE(camera.grab(frame));
    EXPECT_TRUE(frame.valid());
    EXPECT_EQ(frame.width, 64U);
    EXPECT_EQ(frame.height, 48U);
    EXPECT_EQ(frame.sequence, 1U);

    camera.close();
    EXPECT_FALSE(camera.isOpen());
}
