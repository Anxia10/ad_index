#include "gtest/gtest.h"
#include "kernel/store/mmap_store.h"
#include <iostream>
static kernel::store::MMapStore mmap_store;
static kernel::data::Status status;
static kernel::data::Addr addr;
static kernel::data::Data data;
static char buffer[128];
const static std::string test_data = std::string(test_data_dir) + "/store/test/testdata/";

TEST(MMapStore, LogConfig)
{
  LOG_CONFIG(test_data + "log4cpp.properties");
}

TEST(MMapStore, ReadOnlyOpenNotExist)
{
  status = mmap_store.Open(test_data + "mmap_store_test1.dat", true);
  EXPECT_TRUE(!status.operate());
}

TEST(MMapStore, Open)
{
  mmap_store.SetMMapSize(4 * 1024);
  mmap_store.SetMemoryLock(true);
  mmap_store.SetMemoryPreload(true);
  status = mmap_store.Open(test_data + "mmap_store_test.dat");
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, CloseNotReadOnly)
{
  status = mmap_store.Close();
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, ReadOnlyOpenExist)
{
  status = mmap_store.Open(test_data + "mmap_store_test.dat", true);
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, WriteReadOnly)
{
  addr.addr = mmap_store.GetBase();
  std::string test_str = "Write.";
  data.data = const_cast<char *>(test_str.c_str());
  data.len = test_str.size();
  status = mmap_store.Write(addr, data);
  EXPECT_TRUE(!status.operate());
}

TEST(MMapStore, CloseReadOnly)
{
  status = mmap_store.Close();
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, OpenAgain)
{
  status = mmap_store.Open(test_data + "mmap_store_test.dat");
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, Write)
{
  addr.addr = mmap_store.GetBase();
  std::string test_str = "Write.";
  data.data = const_cast<char *>(test_str.c_str());
  data.len = test_str.size();
  status = mmap_store.Write(addr, data);
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, Append)
{
  std::string test_str = "Append.";
  data.data = const_cast<char *>(test_str.c_str());
  data.len = test_str.size();
  status = mmap_store.Append(data);
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, GetSize) {
  size_t expect = 13;
  EXPECT_EQ(expect, mmap_store.GetSize());
}

TEST(MMapStore, WriteBeginOutOfRange) {
  std::string test_str = "OutOfRange.";
  addr.addr = mmap_store.GetBase() - 1;
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  status = mmap_store.Write(addr, data);
  EXPECT_TRUE(!status.operate());
}

TEST(MMapStore, ReadFromBeginToEnd) {
  addr.addr = mmap_store.GetBase();
  data.data = buffer;
  size_t len = mmap_store.GetSize();
  status = mmap_store.Read(addr, len, &data);
  EXPECT_FALSE(!status.operate());
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("Write.Append."), buffer);
}

TEST(MMapStore, ReadFromBeginToMiddle) {
  addr.addr = mmap_store.GetBase();
  data.data = buffer;
  size_t len = mmap_store.GetSize() - 2;
  status = mmap_store.Read(addr, len, &data);
  EXPECT_FALSE(!status.operate());
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("Write.Appen"), buffer);
}

TEST(MMapStore, ReadFromMiddleToEnd) {
  addr.addr = mmap_store.GetBase() + 1;
  data.data = buffer;
  size_t len = mmap_store.GetSize() - 1;
  status = mmap_store.Read(addr, len, &data);
  EXPECT_FALSE(!status.operate());
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("rite.Append."), buffer);
}

TEST(MMapStore, ReadFromMiddleToMiddle) {
  addr.addr = mmap_store.GetBase() + 1;
  data.data = buffer;
  size_t len = mmap_store.GetSize() - 1 - 2;
  status = mmap_store.Read(addr, len, &data);
  EXPECT_FALSE(!status.operate());
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("rite.Appen"), buffer);
}

TEST(MMapStore, Override) {
  addr.addr = mmap_store.GetBase() + 6;
  std::string test_str = "Override.";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  status = mmap_store.Write(addr, data);
  EXPECT_FALSE(!status.operate());
  data.data = buffer;
  addr.addr = mmap_store.GetBase();
  status = mmap_store.Read(addr, mmap_store.GetSize(), &data);
  EXPECT_FALSE(!status.operate());
  memcpy(buffer, data.data, data.len);
  buffer[data.len + 6] = 0;
  EXPECT_EQ(std::string("Write.Override."), buffer);
}

TEST(MMapStore, Expand) {
  size_t old_size = mmap_store.GetSize();
  status = mmap_store.Expand(8 * 1024 - old_size);
  EXPECT_FALSE(!status.operate());
  EXPECT_EQ(8UL * 1024, mmap_store.GetSize());
}

TEST(MMapStore, ReadExceedMMapRange) {
  addr.addr = mmap_store.GetBase() - 1;
  size_t len = 10 * 1024;
  status = mmap_store.Read(addr, len, &data);
  EXPECT_TRUE(!status.operate());

  addr.addr = mmap_store.GetBase() + 2 * 1024;
  len = 3 * 1024;
  status = mmap_store.Read(addr, len, &data);
  EXPECT_TRUE(!status.operate());

  addr.addr = mmap_store.GetBase() + 10 * 1024;
  len = 1;
  status = mmap_store.Read(addr, len, &data);
  EXPECT_TRUE(!status.operate());

  addr.addr = mmap_store.GetBase() + 2 * 1024;
  len = 1 * 1024;
  status = mmap_store.Read(addr, len, &data);
  EXPECT_FALSE(!status.operate());
}

TEST(MMapStore, CloseAgain) {
  status = mmap_store.Close();
  EXPECT_FALSE(!status.operate());
}