#include "gtest/gtest.h"
#include "kernel/store/mmap_store.h"
#include <iostream>
static kernel::store::MMapStore mmap_store;
static kernel::data::Status status;
static kernel::data::Addr addr;
static kernel::data::Data data;
static char buffer[128];
const static std::string base_path = test_data_dir;

TEST(MMapStore, LogConfig) {
    LOG_CONFIG(base_path + "/log4cpp.properties");
}

TEST(MMapStore, Open) {
    mmap_store.SetMMapSize(4 * 1024);
    mmap_store.SetMemoryLock(true);
    mmap_store.SetMemoryPreload(true);
    status = mmap_store.Open(base_path+"/mmap_store_test.dat");
    EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, WriteReadOnly) {
    addr.addr = mmap_store.GetBase();
    std::string test_str = "Write.";
    data.data = const_cast<char*>(test_str.c_str());
    data.len = test_str.size();
    status = mmap_store.Write(addr, data);
    EXPECT_TRUE(!status.operate());
}
