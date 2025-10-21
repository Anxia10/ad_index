#pragma once
#include <vector>
#include "kernel/pool/mmap_pool.h"
#include "kernel/index/index.h"

namespace kernel {
namespace index {
#define SORT_TABLE_KEY_SIZE 8
#if SORT_TABLE_KEY_SIZE == 4
typedef int32_t record_key_type;
#elif SORT_TABLE_KEY_SIZE == 8
typedef int64_t record_key_type;
#endif

struct Record {
  record_key_type key;
  int64_t value;
  void Clear() {
    key = 0L;
    value = 0L;
  }
  bool operator>(int64_t k) {
    return key > k;
  }
  bool operator==(const Record& record) {
    return key == record.key;
  }
  bool operator==(const int64_t k) {
    return key == k;
  }
  void operator=(const Record& r) {
    __sync_lock_test_and_set(&key, r.key);
    __sync_lock_test_and_set(&value, r.value);
  }
  Record& SetKey(int64_t k) {
    __sync_lock_test_and_set(&key, k);
    return *this;
  }
  Record& SetValue(int64_t v) {
    __sync_lock_test_and_set(&value, v);
    return *this;
  }
};

struct SortTableHeader {
  void Init(int64_t init_record_num = 0) {
    used_size = 0;
    total_size = init_record_num;
  }
  int64_t used_size;
  int64_t total_size;
};

enum SortTableType {
  EXCLUSIVE = 1, // 独占的 一般作为一级索引
  SHARED = 2,    // 共享的,作为doclist等
};

class SortTable : public Index {
public:
  SortTable();
  ~SortTable();
  bool Init(pool::MMapPool* mmap_pool, const char* begin = nullptr,
    bool create = true) override;
  bool Insert(const KvPair& kv) override;
  bool Delete(const KvPair& kv) override;
  bool Search(KvPair& kv) override;
  bool BatchInsert(const std::vector<KvPair>& kvs,
    bool already_sorted = false) override;
  bool BatchDelete(const std::vector<KvPair>& kvs,
    bool already_sorted = false) override;
  bool BatchSearch(std::vector<KvPair*>& kvs,
    bool already_sorted = false) override;

  Iterator Begin(int64_t offset = -1L) const override;
  KvPair ConvertElementToKvPair(const Iterator::Element& ele) const override;
  int64_t ConvertElementToDocId(const Iterator::Element& ele) const override {
    Record* r = reinterpret_cast<Record*>(ele.ele);
    return r->value;
  }
  Iterator::Element GetNextElement(const Iterator::Element& ele,
    int32_t n = 1) const override;
  void GetNextElement(Iterator::Element* ele, int32_t n = 1) const override;

  void PrintTable();
  void SetBinarySearchLevel(int32_t level);
  void SetInitRecordNum(int64_t init_record_num) {
    init_record_num_ = init_record_num;
  }
  int64_t GetSize() const override {
    return GetUsedSize();
  }
  int64_t GetInitRecordNum() {
    return init_record_num_;
  }
  int32_t GetBinarySearchLevel() {
    return bs_level_;
  }
  int64_t GetUsedSize() const {
    return header_ == nullptr ? 0L : header_->used_size;
  }
  int64_t GetTotalSize() {
    return header_ == nullptr ? 0L : header_->total_size;
  }
  // 获取倒排链term offset下的使用长度, 即term的倒排链长度
  int64_t GetUsedSize(int64_t offset);
  // 获取倒排链term offset下的总长度, 即空间长度
  int64_t GetTotalSize(int64_t offset);

  int64_t GetTableEntryOffset() override {
    return reinterpret_cast<char*>(header_) -
      mmap_pool_->GetMMapDataBegin();
  }
  void SetIndexType(SortTableType type) {
    type_ = type;
  }

private:
  static int64_t GetHashKey(const void* key, int32_t len);
  bool LoadSortTable(const char* const_begin);
  bool InitEmptySortTable(int64_t init_record_num);
  /*
    -- If search key is found, return record ptr.
       pos will be ignored.
    -- Otherwise, return nullptr.
       If pos isn't nullptr, the index of the
       nearest-less key will be set on pos.
       *pos may be -1 while the search key less
       than the leftest record.
   */
  Record* NormalBinarySearch(const void* key, int32_t len,
    int64_t* pos = nullptr);
  Record* SimdBinarySearch(const void* key, int32_t len,
    int64_t* pos = nullptr);
  Record* DirectlyAdd(int64_t pos, const void* key);
  Record* AllocAdd(int64_t pos, const void* key);
  Record* ExpandAdd(int64_t pos, const void* key);
  Record* GetRecordByKey(const void* key, int32_t len,
    bool allow_create = false, int64_t* ppos = nullptr);

private:
  SortTableType type_ = SortTableType::SHARED;
  int64_t init_record_num_;
  int32_t bs_level_;
  int32_t bs_mid_count_;
  SortTableHeader* header_;
  Record* first_;
  LOG_DECLARE;
};

} // namespace index
} // namespace kernel