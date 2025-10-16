#include "gtest/gtest.h"
#include "kernel/index/hash_table/murmurhash_table.h"

static kernel::index::MurMurHashTable table;
static kernel::pool::MMapPool pool;
static kernel::index::KvPair kv;
const static std::string base_path = test_data_dir;

TEST(MurMurHashTable, LogConfig) {
    LOG_CONFIG(base_path + "/log4cpp.properties");
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
    EXPECT_FALSE(pool.Init(base_path + "/murmurhash_table_test.dat", true));
    pool.Release();
}

TEST(MurMurHashTable, Init) {
    EXPECT_TRUE(pool.Init(base_path + "/murmurhash_table_test.dat"));
    EXPECT_TRUE(table.Init(&pool, pool.GetMMapDataBegin()));
}

TEST(MurMurHashTable, Insert) {
    EXPECT_FALSE(table.Insert(kv.SetKey(nullptr).SetLen(12).SetValue(1)));
    EXPECT_FALSE(table.Insert(kv.SetKey("nihao").SetLen(0).SetValue(1)));
    EXPECT_TRUE(table.Insert(kv.SetKey("nihao").SetLen(5).SetValue(1)));
    EXPECT_EQ(1UL, table.GetSize());
}

// TEST(MurMurHashTable, Search) {

// }