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

TEST(FileStore, OFFT_SIZR) {
  EXPECT_EQ(8UL, sizeof(off_t));
}

TEST(FileStore, ReadOnlyOpenNotExist) {
  status = file_store.Open("file_store_test.dat", true);
  EXPECT_TRUE(!status.operate());
}

TEST(FileStore, Open) {
  status = file_store.Open(base_path + "/file_store_test.dat");
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, CloseNotReadOnly) {
  status = file_store.Close();
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadOnlyOpenExist) {
  status = file_store.Open(base_path + "file_store_test.dat", true);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, WriteReadOnly) {
  addr.addr = 0;
  std::string test_str = "Write.";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  status = file_store.Write(addr, data);
  EXPECT_TRUE(!status.operate());
}

TEST(FileStore, CloseReadOnly) {
  status = file_store.Close();
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, OpenAgain) {
  status = file_store.Open("file_store_test.dat");
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, Write) {
  addr.addr = 0;
  std::string test_str = "Write.";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  status = file_store.Write(addr, data);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, Append) {
  std::string test_str = "Append.";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  status = file_store.Append(data);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, GetSize) {
  size_t expect = 13;
  EXPECT_EQ(expect, file_store.GetSize());
}

TEST(FileStore, WriteBeginOutOfRange) {
  std::string test_str = "OutOfRange.";
  addr.addr = reinterpret_cast<void*>(-1);
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  status = file_store.Write(addr, data);
  EXPECT_TRUE(!status.operate());
}

TEST(FileStore, ReadFromBeginToEnd) {
}

TEST(FileStore, ReadFromBeginToExceedEnd) {
}

// 不是正中间
TEST(FileStore, ReadFromBeginToMiddle) {
}

TEST(FileStore, ReadFromMiddleToEnd) {
}

TEST(FileStore, ReadFromMiddleToExceedEnd) {
}

TEST(FileStore, ReadFromMiddleToMiddle) {
}

TEST(FileStore, ReadOutOfRange) {
}

TEST(FileStore, Override) {
}

TEST(FileStore, Expand) {
}

TEST(FileStore, CloseAgain) {
}


