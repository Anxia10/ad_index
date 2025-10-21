// #include "gtest/gtest.h"
// #include "sophon/index/sort_table/sort_table.h"

// static kernel::index::SortTable table;
// static kernel::pool::MMapPool pool;
// static kernel::index::KvPair kv;
// static int64_t key = 0;
// static int64_t table_entry_offset = 0;
// static std::vector<int64_t> keys;
// static std::vector<kernel::index::KvPair> kvs;
// static int32_t insert_num = 1999;

// TEST(SortTable, LogConfig) {
//   LOG_CONFIG("sophon/index/sort_table/test/testdata/log4cpp.properties");
// }

// TEST(SortTable, CheckStructSize) {
//   EXPECT_EQ(16UL, sizeof(kernel::index::Record));
//   EXPECT_EQ(16UL, sizeof(kernel::index::SortTableHeader));
// }

// TEST(SortTable, InitReadOnly) {
//   EXPECT_FALSE(pool.Init("sort_table_major_test.dat", true));
//   pool.Release();
// }

// TEST(SortTable, Init) {
//   EXPECT_TRUE(pool.Init("sort_table_major_test.dat"));
//   table.SetIndexType(kernel::index::SortTableType::EXCLUSIVE);
//   EXPECT_TRUE(table.Init(&pool, pool.GetMMapDataBegin()));
//   EXPECT_EQ(7, table.GetTotalSize());
// }

// TEST(SortTable, Insert) {
//   EXPECT_FALSE(table.Insert(kv.SetKey(nullptr).SetLen(4).SetValue(1)));
//   key = 2;
//   EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetLen(4).SetValue(2)));
//   EXPECT_EQ(1, table.GetUsedSize());
//   key = 1;
//   EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetLen(4).SetValue(1)));
//   EXPECT_EQ(2048, table.GetUsedSize());
//   EXPECT_EQ(3584, table.GetTotalSize()); // 7*(1<<x)
//   for (int32_t i = 20; i > 0; i--) {
//     key = i;
//     EXPECT_TRUE(table.Search(&kv.SetKey(&key).SetLen(4)));
//     EXPECT_EQ(i, kv.value);
//   }
//   for (int32_t i = 2048; i > 0; i--) {
//     key = i;
//     EXPECT_TRUE(table.Delete(kv.SetKey(&key).SetLen(4)));
//     EXPECT_FALSE(table.Search(&kv.SetKey(&key).SetLen(4)));
//     if (i != 1) {
//       key = i - 1;
//       EXPECT_TRUE(table.Search(&kv.SetKey(&key).SetLen(4)));
//       EXPECT_EQ(i - 1, kv.value);
//     }
//   }
//   EXPECT_EQ(0, table.GetUsedSize());
//   EXPECT_EQ(3584, table.GetTotalSize());
// }

// TEST(SortTable, BatchPrepare) {
//   for (int32_t i = 20480; i > 0; i--) {
//     // Large first.
//     keys.emplace_back(i);
//   }
// }

// TEST(SortTable, BatchInsert) {
//   std::vector<kernel::index::KvPair> l_kvs;
//   for (int32_t i = 4; i > 0; i--) {
//     l_kvs.emplace_back(kernel::index::KvPair(&keys[16 - i], 4, i * 2));
//   }
//   EXPECT_TRUE(table.BatchInsert(l_kvs));
//   EXPECT_EQ(4, table.GetUsedSize());
//   EXPECT_EQ(3584, table.GetTotalSize()); // 7*(1<<x)
//   for (int32_t i = 20480; i > 0; i--) {
//     kvs.emplace_back(kernel::index::KvPair(&keys[20480 - i], 4, i));
//   }
//   EXPECT_TRUE(table.BatchInsert(kvs));
//   EXPECT_EQ(20480, table.GetUsedSize());
//   EXPECT_EQ(20480, table.GetTotalSize());
// }

// TEST(SortTable, BatchClear) {
//   for (int32_t i = 0; i < 20480; i++) {
//     kvs[i].SetValue(0);
//   }
// }

// TEST(SortTable, BatchSearch) {
//   EXPECT_TRUE(table.BatchSearch(&kvs));
//   for (int64_t i = 0; i < 20480; i++) {
//     kv.SetKey(kvs[i].key).SetLen(4);
//     EXPECT_TRUE(table.Search(&kv));
//     EXPECT_EQ(20480 - i, kv.value);
//     EXPECT_EQ(20480 - i,
//               *(reinterpret_cast<
//                 const kernel::index::record_key_type*>(kvs[i].key)));
//     EXPECT_EQ(20480 - i, kvs[i].value);
//   }
// }

// TEST(SortTable, BatchSearchPerf) {
//   table.BatchSearch(&kvs);
// }

// TEST(SortTable, OneByOneSearchPerf) {
//   for (int32_t i = 0; i < 20480; i++) {
//     table.Search(&kvs[20480 - 1 - i]);
//   }
// }

// TEST(SortTable, BatchDelete) {
//   std::vector<kernel::index::KvPair> tmp = {
//     {&keys[234], 4, 0},
//     {&keys[204], 4, 0},
//     {&keys[213], 4, 0},
//     {&keys[113], 4, 0},
//     {&keys[234], 4, 0},
//     {&keys[115], 4, 0}
//   };
//   EXPECT_TRUE(table.BatchDelete(tmp));
//   EXPECT_EQ(20480 - 4, table.GetUsedSize());
//   EXPECT_FALSE(table.Search(&kvs[234]));
//   EXPECT_FALSE(table.Search(&kvs[213]));
//   EXPECT_FALSE(table.Search(&kvs[113]));
//   EXPECT_FALSE(table.Search(&kvs[115]));
//   EXPECT_TRUE(table.Search(&kvs[235]));
//   EXPECT_TRUE(table.Search(&kvs[233]));
//   EXPECT_TRUE(table.BatchDelete(std::vector<kernel::index::KvPair>(
//     kvs.begin(), kvs.begin() + 15240)));
//   EXPECT_EQ(20480 - 15240, table.GetUsedSize());
//   EXPECT_TRUE(table.BatchDelete(std::vector<kernel::index::KvPair>(
//     kvs.begin() + 10240, kvs.end())));
//   EXPECT_TRUE(table.BatchSearch(&kvs));
//   EXPECT_EQ(0, table.GetUsedSize());
//   EXPECT_EQ(20480, table.GetTotalSize());
// }

// TEST(SortTable, Release) {
//   table_entry_offset = table.GetTableEntryOffset();
//   table.Release();
//   pool.Release();
// }

// TEST(SortTable, InitFromFile) {
//   EXPECT_TRUE(pool.Init("sort_table_major_test.dat"));
//   table.SetBinarySearchLevel(3);
//   EXPECT_TRUE(table.Init(&pool,
//     pool.GetMMapDataBegin() + table_entry_offset, false));
//   key = 0;
//   for (int32_t i = 16; i > 0; i--) {
//     key = i;
//     EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetLen(4)
//       .SetValue(i)));
//   }
//   EXPECT_EQ(16, table.GetUsedSize());
//   EXPECT_EQ(20480, table.GetTotalSize());
//   table_entry_offset = table.GetTableEntryOffset();
//   table.Release();
//   kernel::index::SortTable tmp_table;
//   EXPECT_TRUE(tmp_table.Init(&pool,
//     pool.GetMMapDataBegin() + table_entry_offset, false));
//   EXPECT_EQ(16, tmp_table.GetUsedSize());
//   EXPECT_EQ(20480, tmp_table.GetTotalSize());
//   for (int32_t i = 16; i > 0; i--) {
//     key = i;
//     EXPECT_TRUE(tmp_table.Search(&kv.SetKey(&key).SetLen(4)));
//     EXPECT_EQ(key, kv.value);
//   }
//   tmp_table.Release();
// }

// TEST(SortTable, Search) {
//   key = 1;
//   EXPECT_TRUE(table.Search(&kv.SetKey(&key).SetLen(4)));
//   EXPECT_EQ(1U, kv.value);
//   key = 2;
//   EXPECT_TRUE(table.Search(&kv.SetKey(&key).SetLen(4)));
//   EXPECT_EQ(2U, kv.value);
//   EXPECT_FALSE(table.Search(&kv.SetKey(nullptr).SetLen(4)));
//   key = 3;
//   EXPECT_FALSE(table.Search(&kv.SetKey(&key).SetLen(4)));
//   for (int i = 100; i < insert_num; ++i) {
//     int num_key = i;
//     EXPECT_TRUE(table.Search(&kv.SetKey(&num_key).SetLen(4)));
//     EXPECT_EQ(i+1, kv.value);
//   }
// }

// TEST(SortTable, LoadFrom) {
//   pool.Release();
//   table.Release();
//   EXPECT_TRUE(pool.Init("sort_table_major_test.dat"));
//   EXPECT_TRUE(table.Init(&pool, pool.GetMMapDataBegin(), false));
// }

// TEST(SortTable, ReSearch) {
//   key = 1;
//   EXPECT_TRUE(table.Search(&kv.SetKey(&key).SetLen(4)));
// }

// TEST(SortTable, Override) {
//   key = 2;
//   EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetLen(4).SetValue(3)));
//   EXPECT_TRUE(table.Search(&kv));
//   EXPECT_EQ(3U, kv.value);
// }

// TEST(SortTable, Delete) {
//   key = 2;
//   EXPECT_TRUE(table.Delete(kv.SetKey(&key).SetLen(4)));
//   key = 99;
//   EXPECT_TRUE(table.Delete(kv.SetKey(&key).SetLen(4)));
//   EXPECT_EQ(insert_num - 100 + 1, table.GetUsedSize());
//   EXPECT_FALSE(table.Search(&kv));
//   for (int i = 100; i < insert_num; ++i) {
//     int num_key = i;
//     EXPECT_TRUE(table.Delete(kv.SetKey(&num_key).SetLen(4)));
//     EXPECT_FALSE(table.Search(&kv));
//   }
// }

// TEST(SortTable, CheckAllocAdd) {
//   key = 0;
//   for (int32_t i = 2048; i > 0; i--) {
//     key = i;
//     EXPECT_TRUE(table.Insert(kv.SetKey(&key).SetLen(4).SetValue(i)));
//   }
// }