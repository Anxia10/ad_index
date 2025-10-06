#include "kernel/pool/mmap_pool.h"
#include "gtest/gtest.h"
static kernel::pool::MMapPool mmap_pool;
static kernel::data::Status status;
static kernel::data::Addr addr;
static void* alloc_addr = nullptr;
static char buffer[128];
static kernel::data::Data data(buffer);
static const int32_t reuse_timegap = 0;
static const std::string test_str = "MMapPool Test."; // NOLINT
static const size_t test_str_aligned = (test_str.size() + 1UL - 1UL) & ~(1UL - 1UL);

TEST(MMapPool, LogConfig) {
    LOG_CONFIG("sophon/test/testdata/log4cpp.properties");
}

TEST(MMapPool, InitReadOnly) {
    mmap_pool.SetMinExpandSize(1024);
    bool ret = mmap_pool.Init("mmap_pool_test.dat", true);
    EXPECT_FALSE(ret);
}

TEST(MMapPool, Init) {
    bool ret = mmap_pool.Init("mmap_pool_test.dat");
    EXPECT_TRUE(ret);
}

TEST(MMapPool, GetUsedSize) {
    EXPECT_EQ(64UL, mmap_pool.GetUsedSize());
}

TEST(MMapPool, Empty) {
    EXPECT_TRUE(mmap_pool.Empty());
}

TEST(MMapPool, GetDataBegin) {
    EXPECT_EQ(mmap_pool.GetMMapBegin() + 64UL, mmap_pool.GetMMapDataBegin());
}

TEST(MMapPool, Alloc) {
    alloc_addr = mmap_pool.Alloc(test_str.size());
    EXPECT_EQ(mmap_pool.GetMMapBegin() + 64UL,
              reinterpret_cast<char*>(alloc_addr));
    EXPECT_EQ(test_str_aligned + 64UL, mmap_pool.GetUsedSize());
}

TEST(MMapPool, CheckEmpty) {
    EXPECT_FALSE(mmap_pool.Empty());
}

TEST(MMapPool, GetPoolSize) {
    EXPECT_EQ(1024UL, mmap_pool.GetPoolSize());
}

TEST(MMapPool, Write) {
    addr.addr = alloc_addr;
    kernel::data::Data data(const_cast<char*>(test_str.c_str()),
                      test_str.size());
    bool ret = mmap_pool.Write(addr, data);
    EXPECT_TRUE(ret);
}

TEST(MMapPool, Read) {
    bool ret = mmap_pool.Read(addr, test_str.size(), &data);
    EXPECT_TRUE(ret);
    EXPECT_EQ(test_str.size(), data.len);
    memcpy(buffer, data.data, data.len);
    buffer[data.len] = 0;
    EXPECT_EQ(test_str, buffer);
}

TEST(MMapPool, FreeByFreeList) {
    mmap_pool.EnableFreeList();
    mmap_pool.SetReuseTimegap(1);
    bool ret = mmap_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
}

TEST(MMapPool, GetFreeListSize) {
    sleep(2);
    EXPECT_EQ(test_str_aligned, mmap_pool.GetFreeListSize());
}

TEST(MMapPool, DumpFreeListAndReload) {
    mmap_pool.Release();
    EXPECT_TRUE(mmap_pool.Init("mmap_pool_test.dat"));
}

TEST(MMapPool, AllocFromFreeListAfterSeconds) {
    mmap_pool.SetReuseTimegap(reuse_timegap);
    alloc_addr = mmap_pool.Alloc(test_str.size());
    EXPECT_EQ(mmap_pool.GetMMapBegin() + 64UL,
              reinterpret_cast<char*>(alloc_addr));
    EXPECT_EQ(0UL, mmap_pool.GetFreeListSize());
}

TEST(MMapPool, FreeWhileFreeListDisabled) {
    mmap_pool.DisableFreeList();
    bool ret = mmap_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
    EXPECT_EQ(0UL, mmap_pool.GetFreeListSize());
}

TEST(MMapPool, AllocWhileFreeListDisabled) {
    alloc_addr = mmap_pool.Alloc(test_str.size());
    EXPECT_EQ(mmap_pool.GetMMapBegin() + test_str_aligned + 64UL,
              reinterpret_cast<char*>(alloc_addr));
    bool ret = mmap_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
    EXPECT_EQ(0UL, mmap_pool.GetFreeListSize());
}