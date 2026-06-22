#include <gtest/gtest.h>

#include "CameraEngine.hpp"
#include "events/EventBusFactory.hpp"
#include "logging/Logger.hpp"

#include <chrono>
#include <thread>

using csx::camera::CameraEngine;
using csx::camera::CameraSettings;
using csx::core::createEventBus;
using csx::core::createLogger;

TEST(CameraEngineTest, StreamsSyntheticFrames) {
    auto logger = createLogger();
    auto bus = createEventBus();

    CameraSettings settings;
    settings.width = 32;
    settings.height = 24;
    settings.bufferDepth = 4;

    CameraEngine engine(logger, bus);
    engine.configure(settings);
    ASSERT_TRUE(engine.start("synthetic:0"));
    EXPECT_TRUE(engine.isRunning());

    csx::core::Frame frame;
    bool received = false;
    for (int attempt = 0; attempt < 20; ++attempt) {
        if (engine.popFrame(frame, std::chrono::milliseconds{100})) {
            received = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    engine.stop();
    EXPECT_FALSE(engine.isRunning());
    ASSERT_TRUE(received);
    EXPECT_TRUE(frame.valid());
    EXPECT_EQ(frame.width, 32U);
    EXPECT_EQ(frame.height, 24U);
}
