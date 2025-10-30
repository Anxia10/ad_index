#include "gtest/gtest.h"
#include "kernel/index/hash_table/murmurhash_table.h"

static kernel::index::MurMurHashTable table;
static kernel::pool::MMapPool pool;
static kernel::index::KvPair kv;
const static std::string test_data = std::string(test_data_dir) + "/index/testdata";

TEST(MurMurHashTable, LogConfig) {
    LOG_CONFIG(test_data + "/log4cpp.properties");
}

TEST(MurMurHashTable, CheckMurMurHashTableHeader) {
    EXPECT_EQ(64UL, sizeof(kernel::index::MurMurHashTableHeader));
}

TEST(MurMurHashTable, BucketNum) {
    table.SetBucketNum(1);
    EXPECT_EQ(2U, table.GetBucketNum());
    table.SetBucketNum(3);
    EXPECT_EQ(4U, table.GetBucketNum());
    table.SetBucketNum(254);
    EXPECT_EQ(256U, table.GetBucketNum());
    table.SetBucketNum(2);
    EXPECT_EQ(2U, table.GetBucketNum());
}

TEST(MurMurHashTable, InitReadOnly) {
    EXPECT_FALSE(pool.Init(test_data + "/murmurhash_table_test.dat", true));
    pool.Release();
}

TEST(MurMurHashTable, Init) {
    EXPECT_TRUE(pool.Init(test_data + "/murmurhash_table_test.dat"));
    EXPECT_TRUE(table.Init(&pool, pool.GetMMapDataBegin()));
}

TEST(MurMurHashTable, Insert) {
    EXPECT_FALSE(table.Insert(kv.SetKey(nullptr).SetLen(12).SetValue(1)));
    EXPECT_FALSE(table.Insert(kv.SetKey("nihao").SetLen(0).SetValue(1)));
    EXPECT_TRUE(table.Insert(kv.SetKey("nihao").SetLen(5).SetValue(1)));
    EXPECT_EQ(1UL, table.GetSize());
}

TEST(MurMurHashTable, Search) {
    EXPECT_TRUE(table.Search(&kv.SetKey("nihao").SetLen(5)));
    EXPECT_EQ(1U, kv.value);
    EXPECT_FALSE(table.Search(&kv.SetKey("nihao").SetLen(0)));
}

TEST(MurmurHashTable, Search) {
  EXPECT_TRUE(table.Search(&kv.SetKey("nihao").SetLen(5)));
  EXPECT_EQ(1U, kv.value);
  EXPECT_FALSE(table.Search(&kv.SetKey(nullptr).SetLen(5)));
  EXPECT_FALSE(table.Search(&kv.SetKey("nihao").SetLen(0)));
  EXPECT_FALSE(table.Search(&kv.SetKey("nihai").SetLen(5)));
  EXPECT_FALSE(table.Search(&kv.SetKey("nihao").SetLen(6)));
  EXPECT_FALSE(table.Search(&kv.SetKey("nihai").SetLen(6)));
}

TEST(MurmurHashTable, SearchBenchMark) {
  int32_t round = 1000000;
  while (round--) {
    table.Search(&kv.SetKey("nihao").SetLen(5));
  }
}

TEST(MurmurHashTable, Override) {
  EXPECT_TRUE(table.Insert(kv.SetKey("nihao").SetLen(5).SetValue(2)));
  EXPECT_TRUE(table.Search(&kv));
  EXPECT_EQ(2U, kv.value);
  EXPECT_EQ(1L, table.GetSize());
}

TEST(MurmurHashTable, Delete) {
  EXPECT_TRUE(table.Delete(kv.SetKey("nihai").SetLen(5)));
  EXPECT_EQ(1L, table.GetSize());
  EXPECT_TRUE(table.Delete(kv.SetKey("nihao").SetLen(5)));
  EXPECT_EQ(0L, table.GetSize());
  EXPECT_FALSE(table.Search(&kv));
  EXPECT_TRUE(table.Insert(kv.SetValue(3)));
  EXPECT_TRUE(table.Search(&kv));
  EXPECT_EQ(3U, kv.value);
  EXPECT_EQ(1L, table.GetSize());
}

TEST(MurmurHashTable, IteratorTest) {
  EXPECT_TRUE(table.Delete(kv.SetKey("nihao").SetLen(5)));

  std::set<int16_t> insert_set;
  std::set<int16_t> iter_set;
  int16_t key = 0;
  kv.SetLen(sizeof(int16_t));
  for (int16_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetValue(i)));
    insert_set.insert(key);
  }
  for (int16_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Search(&kv.SetKey(&key)));
    EXPECT_EQ(i, kv.value);
  }
  int32_t count = 0;
  for (kernel::index::Index::Iterator iter = table.Begin();
       iter != table.End(); iter++) {
    iter_set.insert(*iter);
    count++;
  }
  EXPECT_EQ(16, count);
  EXPECT_TRUE(insert_set == iter_set);

  for (int16_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Delete(kv.SetKey(&key)));
    EXPECT_FALSE(table.Search(&kv.SetKey(&key)));
  }
  EXPECT_TRUE(table.Insert(kv.SetKey("nihao").SetLen(5).SetValue(3)));
  EXPECT_TRUE(table.Search(&kv));
  EXPECT_EQ(3U, kv.value);
  EXPECT_EQ(1L, table.GetSize());
}

TEST(MurmurHashTable, CheckWhileHashCrash) {
  int16_t key = 0;
  kv.SetLen(sizeof(int16_t));
  for (int16_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetValue(i)));
  }
  for (int16_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Search(&kv.SetKey(&key)));
    EXPECT_EQ(i, kv.value);
  }
  for (int16_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Delete(kv.SetKey(&key)));
    EXPECT_FALSE(table.Search(&kv.SetKey(&key)));
    if (i != 15) {
      key = i + 1;
      EXPECT_TRUE(table.Search(&kv.SetKey(&key)));
      EXPECT_EQ(i + 1, kv.value);
    }
  }
}

TEST(MurmurHashTable, CheckWhileHashCrashAndLargeKey) {
  static const uint32_t kKeySize = 60;
  int32_t key_buffer[kKeySize];
  int32_t* key = key_buffer;
  kv.SetLen(sizeof(key_buffer));
  for (int32_t i = 0; i < 16; i++) {
    *key = i;
    EXPECT_TRUE(table.Insert(kv.SetKey(key).SetValue(i)));
  }
  for (int32_t i = 0; i < 16; i++) {
    *key = i;
    EXPECT_TRUE(table.Search(&kv.SetKey(key)));
    EXPECT_EQ(static_cast<uint32_t>(i), kv.value);
  }
  for (int32_t i = 0; i < 16; i++) {
    *key = i;
    EXPECT_TRUE(table.Delete(kv.SetKey(key)));
    EXPECT_FALSE(table.Search(&kv.SetKey(key)));
    if (i != 15) {
      *key = i + 1;
      EXPECT_TRUE(table.Search(&kv.SetKey(key)));
      EXPECT_EQ(static_cast<uint32_t>(i + 1), kv.value);
    }
  }
}

TEST(MurmurHashTable, Release) {
  table.Release();
  pool.Release();
}

TEST(MurmurHashTable, InitFromFile) {
  EXPECT_TRUE(pool.Init(test_data + "/murmurhash_table_test.dat"));
  table.SetBucketNum(15);
  EXPECT_TRUE(table.Init(&pool, pool.GetMMapDataBegin(), false));
  EXPECT_EQ(2U, table.GetBucketNum());
  int32_t key = 0;
  kv.SetLen(sizeof(int32_t));
  for (int32_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetValue(1)));
  }
  table.Release();
  table.SetBucketNum(15);
  EXPECT_TRUE(table.Init(&pool, pool.GetMMapDataBegin(), false));
  EXPECT_EQ(2U, table.GetBucketNum());
  for (int32_t i = 0; i < 16; i++) {
    key = i;
    EXPECT_TRUE(table.Search(&kv.SetKey(&key)));
    EXPECT_EQ(static_cast<uint32_t>(key), kv.value);
  }
  table.Release();
}