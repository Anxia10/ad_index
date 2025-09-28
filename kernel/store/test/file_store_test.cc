#include "gtest/gtest.h"
#include "kernel/store/file_store.h"
static kernel::store::FileStore file_store;
static kernel::data::Status status;
static kernel::data::Addr addr;
static kernel::data::Data data;
static char buffer[128];

TEST(FileStore, write){
    addr.addr = 0;
    std::string test_str = "Write.";
    data.data = const_cast<char*>(test_str.c_str());
    data.len = test_str.size();
    status = file_store.Write(addr, data);
    EXPECT_FALSE(!status.operate());
}