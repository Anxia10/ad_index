#include "kernel/pool/file_pool.h"
#include "gtest/gtest.h"
static kernel::pool::FilePool file_pool;
static kernel::data::Status status;
static kernel::data::Addr addr;
static void* alloc_addr = nullptr;
static char buffer[128];
static kernel::data::Data data(buffer);
static const std::string test_str = "FilePool Test."; // NOLINT
static const int32_t reuse_timegap = 0;

TEST(FilePool, LogConfig) {
    LOG_CONFIG("sophon/test/testdata/log4cpp.properties");
}

TEST(FilePool, Size) {
    EXPECT_EQ(16U, sizeof(kernel::pool::Block));
}

TEST(FilePool, InitReadOnly) {
    bool ret = file_pool.Init("file_pool_test.dat", true);
    EXPECT_FALSE(ret);
}

TEST(FilePool, Init) {
    bool ret = file_pool.Init("file_pool_test.dat");
    EXPECT_TRUE(ret);
}

TEST(FilePool, Alloc) {
    alloc_addr = file_pool.Alloc(test_str.size());
    EXPECT_EQ(64UL, reinterpret_cast<size_t>(alloc_addr));
}

TEST(FilePool, GetPoolSize) {
    EXPECT_EQ(test_str.size() + 64UL, file_pool.GetPoolSize());
}

TEST(FilePool, Write) {
    addr.addr = alloc_addr;
    kernel::data::Data data(const_cast<char*>(test_str.c_str()),
                      test_str.size());
    bool ret = file_pool.Write(addr, data);
    EXPECT_TRUE(ret);
}

TEST(FilePool, GetFreeListSize) {
    sleep(2);
    EXPECT_EQ(test_str.size(), file_pool.GetFreeListSize());
}

TEST(FilePool, DumpFreeListAndReload) {
    file_pool.Release();
    EXPECT_TRUE(file_pool.Init("file_pool_test.dat"));
}

TEST(FilePool, AllocFromFreeListAfterSeconds) {
    file_pool.SetReuseTimegap(reuse_timegap);
    alloc_addr = file_pool.Alloc(test_str.size());
    EXPECT_EQ(test_str.size() + 64, file_pool.GetPoolSize());
    EXPECT_EQ(0UL, file_pool.GetFreeListSize());
}

TEST(FilePool, FreeWhileFreeListDisabled) {
    file_pool.DisableFreeList();
    bool ret = file_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
    EXPECT_EQ(0UL, file_pool.GetFreeListSize());
}

TEST(FilePool, AllocWhileFreeListDisabled) {
    alloc_addr = file_pool.Alloc(test_str.size());
    EXPECT_EQ(test_str.size() + 64,
              reinterpret_cast<size_t>(alloc_addr));
    EXPECT_EQ(test_str.size() * 2 + 64,
              file_pool.GetPoolSize());
    bool ret = file_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
    EXPECT_EQ(0UL, file_pool.GetFreeListSize());
}

TEST(FilePool, Read) {
    bool ret = file_pool.Read(addr, test_str.size(), &data);
    EXPECT_TRUE(ret);
    EXPECT_EQ(test_str.size(), data.len);
    buffer[data.len] = 0;
    EXPECT_EQ(test_str, reinterpret_cast<const char*>(data.data));
}

TEST(FilePool, FreeByFreeList) {
    file_pool.EnableFreeList();
    file_pool.SetReuseTimegap(1);
    bool ret = file_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
}

TEST(FilePool, GetFreeListSize) {
    sleep(2);
    EXPECT_EQ(test_str.size(), file_pool.GetFreeListSize());
}

TEST(FilePool, DumpFreeListAndReload) {
    file_pool.Release();
    EXPECT_TRUE(file_pool.Init("file_pool_test.dat"));
}

TEST(FilePool, AllocFromFreeListAfterSeconds) {
    file_pool.SetReuseTimegap(reuse_timegap);
    alloc_addr = file_pool.Alloc(test_str.size());
    EXPECT_EQ(test_str.size() + 64, file_pool.GetPoolSize());
    EXPECT_EQ(0UL, file_pool.GetFreeListSize());
}

TEST(FilePool, FreeWhileFreeListDisabled) {
    file_pool.DisableFreeList();
    bool ret = file_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
    EXPECT_EQ(0UL, file_pool.GetFreeListSize());
}

TEST(FilePool, AllocWhileFreeListDisabled) {
    alloc_addr = file_pool.Alloc(test_str.size());
    EXPECT_EQ(test_str.size() + 64,
              reinterpret_cast<size_t>(alloc_addr));
    EXPECT_EQ(test_str.size() * 2 + 64,
              file_pool.GetPoolSize());
    bool ret = file_pool.Free(alloc_addr, test_str.size());
    EXPECT_TRUE(ret);
    EXPECT_EQ(0UL, file_pool.GetFreeListSize());
}