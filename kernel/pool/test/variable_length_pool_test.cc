/**
 Copyright [2021-1-11] <xuyao>
 */
#include "gtest/gtest.h"
#include "kernel/pool/variable_length_pool.h"

static kernel::pool::VariableLengthPool variable_length_pool;
static kernel::data::Addr addr;
static kernel::data::Data data;
static char buffer[128];
const static std::string test_data = std::string(test_data_dir) + "/pool/test/testdata";

TEST(VariableLengthPool, LogConfig) {
  LOG_CONFIG(test_data + "/log4cpp.properties");
}

TEST(VariableLengthPool, ReadOnlyInitNotExist) {
  EXPECT_FALSE(variable_length_pool.Init(test_data + "/variable_length_pool_test.dat", true));
}

TEST(VariableLengthPool, Init) {
  EXPECT_TRUE(variable_length_pool.Init(test_data + "/variable_length_pool_test.dat"));
}

TEST(VariableLengthPool, ReleaseNotReadOnly) {
  variable_length_pool.Release();
}

TEST(VariableLengthPool, ReadOnlyInitExist) {
  EXPECT_TRUE(variable_length_pool.Init(test_data + "/variable_length_pool_test.dat", true));
}

TEST(VariableLengthPool, ReleaseReadOnly) {
  variable_length_pool.Release();
}

TEST(VariableLengthPool, InitAgain) {
  EXPECT_TRUE(variable_length_pool.Init(test_data + "/variable_length_pool_test.dat"));
}

TEST(VariableLengthPool, GetDataSize) {
  EXPECT_EQ(0UL, variable_length_pool.GetDataSize());
}

TEST(VariableLengthPool, NewData) {
  std::string test_str = "Append.";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_NE(-1, variable_length_pool.NewData(data));
  EXPECT_EQ(11UL, variable_length_pool.GetDataSize());
}

TEST(VariableLengthPool, Write) {
  addr.addr = variable_length_pool.GetMMapDataBegin();
  std::string test_str = "!!!!!!!!!!!";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  // size too large
  EXPECT_FALSE(variable_length_pool.Write(addr, data));
  test_str = "Write.";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_TRUE(variable_length_pool.Write(addr, data));
}

TEST(VariableLengthPool, Read) {
  addr.addr = variable_length_pool.GetMMapDataBegin();
  data.data = buffer;
  EXPECT_TRUE(variable_length_pool.Read(addr, 0, &data));
  EXPECT_EQ(6UL, data.len);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("Write."), buffer);
}

TEST(VariableLengthPool, AppendAnother) {
  int32_t offset = variable_length_pool.GetDataSize();
  EXPECT_EQ(11, offset);
  std::string test_str = "Append.Append.";
  data.data = const_cast<char*>(test_str.c_str());
  data.len = test_str.size();
  EXPECT_TRUE(variable_length_pool.NewData(data));
  EXPECT_EQ(29UL, variable_length_pool.GetDataSize());
  addr.addr = variable_length_pool.GetMMapDataBegin() + offset;
  EXPECT_TRUE(variable_length_pool.Read(addr, 0, &data));
  EXPECT_EQ(14UL, data.len);
  memcpy(buffer, data.data, data.len);
  buffer[data.len] = 0;
  EXPECT_EQ(std::string("Append.Append."), buffer);
}

TEST(VariableLengthPool, ReleaseAgain) {
  variable_length_pool.Release();
}