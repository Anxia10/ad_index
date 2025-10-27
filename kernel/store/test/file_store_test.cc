#include <iostream>
#include "gtest/gtest.h"
#include "kernel/store/file_store.h"
static kernel::store::FileStore file_store;
static kernel::data::Status status;
static kernel::data::Addr addr;
static kernel::data::Data data;
static char buffer[128];
const static std::string test_data = std::string(test_data_dir) + "/store/test/testdata";

TEST(FileStore, LogConfig) {
  LOG_CONFIG(test_data + "/log4cpp.properties");
}

TEST(FileStore, OFFT_SIZR) {
  EXPECT_EQ(8UL, sizeof(off_t));
}

TEST(FileStore, ReadOnlyOpenNotExist) {
  status = file_store.Open("/file_store_test.dat", true);
  EXPECT_TRUE(!status.operate());
}

TEST(FileStore, Open) {
  status = file_store.Open(test_data + "/file_store_test.dat");
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, CloseNotReadOnly) {
  status = file_store.Close();
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadOnlyOpenExist) {
  status = file_store.Open(test_data + "/file_store_test.dat", true);
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
  status = file_store.Open(test_data + "/file_store_test.dat");
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
  // std::cout<< reinterpret_cast<char*>(const_cast<void*>(data.data)) << std::endl;
  addr.addr = reinterpret_cast<void*>(0);
  status = file_store.Read(addr, file_store.GetSize(), &data);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(13, data.len);
  EXPECT_EQ(std::string("Write.Append.") , buffer);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadFromBeginToExceedEnd) {
  addr.addr = reinterpret_cast<void*>(0);
  status = file_store.Read(addr, file_store.GetSize()+1, &data);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(13, data.len);
  EXPECT_EQ(std::string("Write.Append.") , buffer);
  EXPECT_FALSE(!status.operate());
}

// // 不是正中间
TEST(FileStore, ReadFromBeginToMiddle) {
  addr.addr = reinterpret_cast<void*>(0);
  status = file_store.Read(addr, file_store.GetSize()-2, &data);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(11, data.len);
  EXPECT_EQ(std::string("Write.Appen") , buffer);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadFromMiddleToEnd) {
  addr.addr = reinterpret_cast<void*>(2);
  status = file_store.Read(addr, file_store.GetSize(), &data);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(11, data.len);
  EXPECT_EQ(std::string("ite.Append.") , buffer);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadFromMiddleToExceedEnd) {
  addr.addr = reinterpret_cast<void*>(2);
  status = file_store.Read(addr, file_store.GetSize()+1, &data);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(11, data.len);
  EXPECT_EQ(std::string("ite.Append.") , buffer);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadFromMiddleToMiddle) {
  addr.addr = reinterpret_cast<void*>(2);
  status = file_store.Read(addr, file_store.GetSize()-4, &data);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(9, data.len);
  EXPECT_EQ(std::string("ite.Appen") , buffer);
  EXPECT_FALSE(!status.operate());
}

TEST(FileStore, ReadOutOfRange) {
  addr.addr = reinterpret_cast<void*>(99);
  status = file_store.Read(addr, file_store.GetSize(), &data);
  EXPECT_TRUE(!status.operate());
}

TEST(FileStore, Override) {
  addr.addr = reinterpret_cast<void*>(6);
  std::string test_str = "Override.";
  data.data = test_str.c_str();
  data.len = test_str.size();
  status = file_store.Write(addr, data);
  EXPECT_FALSE(!status.operate());
  data.data = buffer;
  addr.addr = 0;
  status = file_store.Read(addr, 100, &data);
  EXPECT_FALSE(!status.operate());
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("Write.Override.") , reinterpret_cast<const char*>(data.data));
}

TEST(FileStore, Expand) {
  size_t size = file_store.GetSize();
  status = file_store.Expand(1);
  EXPECT_EQ(size + 1, file_store.GetSize());
  EXPECT_FALSE(!status.operate());
}

// TEST(FileStore, Truncate) {
//   status = file_store.Expand(10);
//   EXPECT_EQ(10, file_store.GetSize());
//   EXPECT_FALSE(!status.operate());
// }

TEST(FileStore, CloseAgain) {
  status = file_store.Close();
  EXPECT_FALSE(!status.operate());
}


