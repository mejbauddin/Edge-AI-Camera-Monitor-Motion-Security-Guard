#include <gtest/gtest.h>

#include "threading/RingBuffer.hpp"

using csx::core::RingBuffer;

TEST(RingBufferTest, PushPopMaintainsOrder) {
    RingBuffer<int, 4> buffer;
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));

    int value = 0;
    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 2);
    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 3);
    EXPECT_FALSE(buffer.pop(value));
}

TEST(RingBufferTest, FullBufferRejectsPush) {
    RingBuffer<int, 2> buffer;
    EXPECT_TRUE(buffer.push(10));
    EXPECT_TRUE(buffer.push(20));
    EXPECT_FALSE(buffer.push(30));
    EXPECT_EQ(buffer.size(), 2U);
}

TEST(RingBufferTest, ClearResetsState) {
    RingBuffer<int, 3> buffer;
    buffer.push(1);
    buffer.push(2);
    buffer.clear();
    EXPECT_TRUE(buffer.empty());
    EXPECT_TRUE(buffer.push(99));
}
