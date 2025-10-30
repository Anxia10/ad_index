#include "gtest/gtest.h"
#include "kernel/index/hash_table/hash_bucket.h"

kernel::index::HashBucket bucket;
static char _buffer[128];
static char* buffer = _buffer;

TEST(HashBucket, Size) {
    EXPECT_EQ(17UL, sizeof(kernel::index::HashBucket)); // 17个字节
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

TEST(HashBucket, SetAndCheckHashCodeInPayLod) {
  bucket.SetHashCodeInPayload(1);
  EXPECT_TRUE(bucket.HashCodeInPayload());
  EXPECT_TRUE(bucket.CheckHashCodeInPayload(1));
  EXPECT_FALSE(bucket.CheckHashCodeInPayload(2));
}

TEST(HashBucket, SetAndCheckKeyInPayLodLt16) {
  bucket.Clear();
  for (int32_t i = 0; i < 15; i++) {
    buffer[i] = i;
  }
  bucket.SetKeyInPayload(buffer, 15);
  EXPECT_TRUE(bucket.KeyInPayload());
  EXPECT_TRUE(bucket.CheckKeyInPayload(buffer, 15));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer, 16));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer + 1, 15));
}

TEST(HashBucket, SetAndCheckKeyInPayLodLt32) {
  bucket.Clear();
  for (int32_t i = 0; i < 30; i++) {
    buffer[i] = i;
  }
  bucket.SetKeyInPayload(buffer, 30);
  EXPECT_TRUE(bucket.KeyInPayload());
  EXPECT_TRUE(bucket.CheckKeyInPayload(buffer, 30));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer, 29));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer + 1, 30));
}

TEST(HashBucket, SetAndCheckKeyInPayLodLt46) {
  bucket.Clear();
  for (int32_t i = 0; i < 36; i++) {
    buffer[i] = i;
  }
  bucket.SetKeyInPayload(buffer, 36);
  EXPECT_TRUE(bucket.KeyInPayload());
  EXPECT_TRUE(bucket.CheckKeyInPayload(buffer, 36));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer, 35));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer + 1, 36));
}

TEST(HashBucket, SetAndCheckKeyInPayLodEq46) {
  bucket.Clear();
  for (int32_t i = 0; i < 46; i++) {
    buffer[i] = i;
  }
  bucket.SetKeyInPayload(buffer, 46);
  EXPECT_TRUE(bucket.KeyInPayload());
  EXPECT_TRUE(bucket.CheckKeyInPayload(buffer, 46));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer, 47));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer + 1, 46));
}

TEST(HashBucket, SetAndCheckKeyInPayLodGt46) {
  bucket.Clear();
  for (int32_t i = 0; i < 60; i++) {
    buffer[i] = i;
  }
  bucket.SetKeyInPayload(buffer, 60);
  EXPECT_TRUE(bucket.KeyInPayload());
  EXPECT_TRUE(bucket.CheckKeyInPayload(buffer, 60));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer, 55));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer + 1, 60));
}

TEST(HashBucket, SetAndCheckKeyInPayLodGt46FalsePositive) {
  bucket.Clear();
  for (int32_t i = 0; i < 60; i++) {
    buffer[i] = i;
  }
  bucket.SetKeyInPayload(buffer, 60);
  EXPECT_TRUE(bucket.KeyInPayload());
  for (int32_t i = 0; i < 10; i++) {
    buffer[46 + i] = i;
  }
  // false positive
  EXPECT_TRUE(bucket.CheckKeyInPayload(buffer, 60));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer, 55));
  EXPECT_FALSE(bucket.CheckKeyInPayload(buffer + 1, 60));
}