#include "gtest/gtest.h"
#include "kernel/pool/fix_length_pool.h"

static kernel::pool::FixLengthPool fix_length_pool(5);
static kernel::data::Addr addr;
static kernel::data::Data data;
static char buffer[128];

TEST(FixLengthPool, LogConfig) {
  LOG_CONFIG("sophon/test/testdata/log4cpp.properties");
}

TEST(FixLengthPool, ReadOnlyInitNotExist) {
  EXPECT_FALSE(fix_length_pool.Init(
      "fix_length_pool_test.dat", true));
}

TEST(FixLengthPool, Init) {
  EXPECT_TRUE(fix_length_pool.Init("fix_length_pool_test.dat"));
}

TEST(FixLengthPool, ReleaseNotReadOnly) {
  fix_length_pool.Release();
}

TEST(FixLengthPool, ReadOnlyInitExist) {
  EXPECT_TRUE(fix_length_pool.Init("fix_length_pool_test.dat", true));
}

TEST(FixLengthPool, ReleaseReadOnly) {
  fix_length_pool.Release();
}

TEST(FixLengthPool, InitAgain) {
  EXPECT_TRUE(fix_length_pool.Init("fix_length_pool_test.dat"));
}

TEST(FixLengthPool, GetDataSize) {
  EXPECT_EQ(0UL, fix_length_pool.GetDataSize());
}

TEST(FixLengthPool, NewData) {
  std::string test_str;

  test_str = "xxxxx";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_EQ(-1L, fix_length_pool.NewData(data));

  test_str = "xxxx";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_EQ(-1L, fix_length_pool.NewData(data));

  test_str = "xxxxx";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_NE(-1L, fix_length_pool.NewData(data));
  EXPECT_EQ(5UL, fix_length_pool.GetDataSize());
}

TEST(FixLengthPool, Write) {
  addr.addr = fix_length_pool.GetMMapDataBegin();
  std::string test_str;

  // size too large
  test_str = "yyyyyy";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_FALSE(fix_length_pool.Write(addr, data));

  // size too small
  test_str = "yyyy";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_FALSE(fix_length_pool.Write(addr, data));

  test_str = "yyyyy";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_TRUE(fix_length_pool.Write(addr, data));
}

TEST(FixLengthPool, Read) {
  addr.addr = fix_length_pool.GetMMapDataBegin();
  data.data = buffer;
  EXPECT_TRUE(fix_length_pool.Read(addr, 0, &data));
  EXPECT_EQ(5UL, data.len);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("yyyyy"), buffer);
}

TEST(FixLengthPool, AppendAnother) {
  int32_t offset = fix_length_pool.GetDataSize();
  EXPECT_EQ(5, offset);
  std::string test_str = "zzzzz";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_TRUE(fix_length_pool.NewData(data));
  EXPECT_EQ(10UL, fix_length_pool.GetDataSize());
  addr.addr = fix_length_pool.GetMMapDataBegin() + offset;
  EXPECT_TRUE(fix_length_pool.Read(addr, 0, &data));
  EXPECT_EQ(5UL, data.len);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("zzzzz"), buffer);
}

TEST(FixLengthPool, ReleaseAgain) {
  fix_length_pool.Release();
}