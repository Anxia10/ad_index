#include "gtest/gtest.h"
#include "kernel/index/hash_table/hash_bucket.h"

kernel::index::HashBucket bucket;
static char _buffer[128];
static char* buffer = _buffer;

TEST(HashBucket, Size) {
    EXPECT_EQ(16UL, sizeof(kernel::index::HashBucket));
}

TEST(HashBucket, CheckInitParam) {
    EXPECT_FALSE(bucket.BucketOccupied());
    EXPECT_FALSE(bucket.HashCodeInPayload());
    EXPECT_FALSE(bucket.KeyInPayload());
}

TEST(HashBucket, MarkAndCheckParam) {
    bucket.MarkBucketOccupied();
    bucket.MarkHashCodeInPayload();
    bucket.MarkKeyInPayload();
    EXPECT_TRUE(bucket.BucketOccupied());
    EXPECT_TRUE(bucket.HashCodeInPayload());
    EXPECT_TRUE(bucket.KeyInPayload());
}

TEST(HashBucket, ClearAndCheckParam) {
    bucket.ClearBucketOccupied();
    bucket.ClearHashCodeInPayload();
    bucket.ClearKeyInPayload();
    EXPECT_FALSE(bucket.BucketOccupied());
    EXPECT_FALSE(bucket.HashCodeInPayload());
    EXPECT_FALSE(bucket.KeyInPayload());
}

TEST(HashBucket, Clear) {
    bucket.MarkBucketOccupied();
    bucket.MarkHashCodeInPayload();
    bucket.MarkKeyInPayload();
    bucket.Clear();
    EXPECT_FALSE(bucket.BucketOccupied());
    EXPECT_FALSE(bucket.HashCodeInPayload());
    EXPECT_FALSE(bucket.KeyInPayload());
}