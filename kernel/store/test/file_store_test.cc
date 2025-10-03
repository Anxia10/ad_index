#include "gtest/gtest.h"
#include <iostream>
#include "kernel/store/file_store.h"
static kernel::store::FileStore file_store;
static kernel::data::Status status;
static kernel::data::Addr addr;
static kernel::data::Data data;
static char buffer[128];
const static std::string base_path = test_data_dir;

TEST(FileStore, LogConfig) {
    LOG_CONFIG(base_path + "/log4cpp.properties");
}

TEST(FileStore, Open) {
    status = file_store.Open("file_store_test.dat", true);
    EXPECT_TRUE(!status.operate());
}

TEST(FileStore, Open2) {
    status = file_store.Open(base_path + "/file_store_test.dat", true);
    EXPECT_FALSE(!status.operate());
}

TEST(FileStore, CLOSE) {
    status = file_store.Close();
    EXPECT_FALSE(!status.operate());
}


TEST(FileStore, OpenNotOnlyRead) {
    status = file_store.Open(base_path + "/file_store_test.dat");
    EXPECT_FALSE(!status.operate());
}


TEST(FileStore, WRITE){
    addr.addr = 0;
    std::string test_str = "Write.";
    data.data = const_cast<char*>(test_str.c_str());
    data.len = test_str.size();
    status = file_store.Write(addr, data);
    EXPECT_FALSE(!status.operate());
}

TEST(FileStore, Append){
    addr.addr = 0;
    std::string test_str = "Append.";
    data.data = const_cast<char*>(test_str.c_str());
    data.len = test_str.size();
    status = file_store.Append(data);
    EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadFromBeginToEnd){
    addr.addr = 0;
    data.data = buffer;
    size_t len = file_store.GetSize();
    status = file_store.Read(addr, len, &data);
    EXPECT_FALSE(!status.operate());
    buffer[data.len] = 0;
    EXPECT_EQ(std::string("Write.Append."), reinterpret_cast<const char*>(data.data));
}

TEST(FileStore, OpenNotOnlyReadClose) {
    status = file_store.Close();
    EXPECT_FALSE(!status.operate());
}