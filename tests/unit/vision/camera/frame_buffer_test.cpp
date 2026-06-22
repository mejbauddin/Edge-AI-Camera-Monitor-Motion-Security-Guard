#include <gtest/gtest.h>

#include "FrameBuffer.hpp"

#include <chrono>

using csx::camera::FrameBuffer;

TEST(FrameBufferTest, DropsOldestWhenFull) {
    FrameBuffer buffer(2);
    csx::core::Frame frameA;
    frameA.sequence = 1;
    csx::core::Frame frameB;
    frameB.sequence = 2;
    csx::core::Frame frameC;
    frameC.sequence = 3;

    buffer.push(frameA);
    buffer.push(frameB);
    buffer.push(frameC);

    EXPECT_EQ(buffer.droppedFrames(), 1U);

    csx::core::Frame out;
    ASSERT_TRUE(buffer.pop(out));
    EXPECT_EQ(out.sequence, 2U);
}

TEST(FrameBufferTest, PopTimesOutWhenEmpty) {
    FrameBuffer buffer(2);
    csx::core::Frame out;
    EXPECT_FALSE(buffer.pop(out, std::chrono::milliseconds{10}));
}
