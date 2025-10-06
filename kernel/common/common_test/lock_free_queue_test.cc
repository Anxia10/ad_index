#include "gtest/gtest.h"
#include "kernel/common/concurrency/lock_free_queue.hpp"

kernel::concurrency::LockFreeQueue<int32_t> queue(5);

TEST(LockFreeQueue, Size) {
    EXPECT_EQ(0UL, queue.Size());
}

TEST(LockFreeQueue, Empty) {
    EXPECT_TRUE(queue.Empty());
}

TEST(LockFreeQueue, Push) {
    EXPECT_TRUE(queue.Push(1));
    EXPECT_TRUE(queue.Push(2));
    EXPECT_TRUE(queue.Push(3));
    EXPECT_TRUE(queue.Push(4));
    EXPECT_TRUE(queue.Push(5));
    EXPECT_FALSE(queue.Push(6));
    EXPECT_EQ(5UL, queue.Size());
    EXPECT_FALSE(queue.Empty());
}

TEST(LockFreeQueue, Pop) {
    int32_t r = 0;
    EXPECT_TRUE(queue.Pop(&r));
    EXPECT_EQ(1, r);
    EXPECT_TRUE(queue.Pop(&r));
    EXPECT_EQ(2, r);
    EXPECT_TRUE(queue.Pop(&r));
    EXPECT_EQ(3, r);
    EXPECT_TRUE(queue.Pop(&r));
    EXPECT_EQ(4, r);
    EXPECT_TRUE(queue.Pop(&r));
    EXPECT_EQ(5, r);
    EXPECT_TRUE(queue.Empty());
    EXPECT_EQ(0UL, queue.Size());
    EXPECT_FALSE(queue.Pop(&r));
}