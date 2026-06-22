#include <gtest/gtest.h>

#include "CameraTypes.hpp"

using csx::camera::parseSourceUri;

TEST(CameraTypesTest, ParsesUsbSource) {
    const auto info = parseSourceUri("usb:1");
    EXPECT_EQ(info.deviceIndex, 1);
}

TEST(CameraTypesTest, ParsesRtspSource) {
    const auto info = parseSourceUri("rtsp://192.168.1.10/stream");
    EXPECT_EQ(static_cast<int>(info.type), 1);
}

TEST(CameraTypesTest, ParsesHttpIpSource) {
    const auto info = parseSourceUri("http://192.168.1.20/mjpeg");
    EXPECT_EQ(static_cast<int>(info.type), 2);
}
