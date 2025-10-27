#include <immintrin.h>
#include <avxintrin.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include "kernel/index/sort_table/sort_table.h"
#include "kernel/common/utils/utils.h"
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace index {

static const int64_t kDefaultInitRecordNum = 7;
#if SORT_TABLE_KEY_SIZE == 4
static const int32_t kMaxBinarySearchLevel = 3;
static const int32_t kDefaultBinarySearchMidCount = 7;
#elif SORT_TABLE_KEY_SIZE == 8
static const int32_t kMaxBinarySearchLevel = 2;
static const int32_t kDefaultBinarySearchMidCount = 3;
#endif
static const uint32_t kHighHashSeed = 0x89741511;
static const uint32_t kLowHashSeed = 0x15118974;

LOG_SETUP("kernel", SortTable);

SortTable::SortTable() :
  init_record_num_(kDefaultInitRecordNum),
  bs_level_(kMaxBinarySearchLevel),
  bs_mid_count_(kDefaultBinarySearchMidCount),
  header_(nullptr), first_(nullptr) {
}

SortTable::~SortTable() {
}

void SortTable::SetBinarySearchLevel(int32_t level) {
  bs_level_ = level > kMaxBinarySearchLevel ?
    kMaxBinarySearchLevel : level;
  bs_mid_count_ = (1 << bs_level_) - 1;
}

bool SortTable::Init(pool::MMapPool* mmap_pool,
  const char* begin, bool create) {
  if (unlikely(mmap_pool == nullptr)) {
    LOG_ERROR("MMapPool [%p] or begin [%p] is nullptr.",
      mmap_pool, begin);
    return false;
  }
  mmap_pool_ = mmap_pool;
  if (!create) {
    if (!LoadSortTable(begin)) {
      LOG_ERROR("Load sorttable fail.");
      return false;
    }
  } else {
    if (!InitEmptySortTable(init_record_num_)) {
      LOG_ERROR("Init empty sorttable fail.");
      return false;
    }
  }
  return true;
}

bool SortTable::LoadSortTable(const char* const_begin) {
  char* begin = const_cast<char*>(const_begin);
  header_ = reinterpret_cast<SortTableHeader*>(begin);
  if (header_ == nullptr) {
    LOG_ERROR("Begin addr is nullptr.");
    return false;
  }
  first_ = reinterpret_cast<Record*>(header_ + 1);
  return true;
}

bool SortTable::InitEmptySortTable(int64_t init_record_num) {
  if (unlikely(mmap_pool_ == nullptr)) {
    LOG_ERROR("MMapPool is null.");
    return false;
  }
  void* addr = mmap_pool_->Alloc(sizeof(SortTableHeader) +
    sizeof(Record) * init_record_num);
  header_ = reinterpret_cast<SortTableHeader*>(addr);
  if (unlikely(header_ == nullptr)) {
    LOG_ERROR("MMap alloc header fail.");
    return false;
  }
  header_->Init(init_record_num);
  first_ = reinterpret_cast<Record*>(header_ + 1);
  for (int64_t i = 0; i < init_record_num; i++) {
    first_[i].Clear();
  }
  return true;
}

inline Record* SortTable::SimdBinarySearch(
    const void* key, int32_t len, int64_t* pos) {
    int64_t size = header_->used_size;
    if (size <= 0L) {
        if (pos != nullptr) *pos = -1L;
        return nullptr;
    }
    int32_t last_mid = bs_mid_count_ - 1;
    int64_t left = 0;
    int64_t right = header_->used_size - 1;
    std::vector<int32_t> mid;
    mid.resize(bs_mid_count_);
    int64_t base = 0;
    int64_t rem = 0;
    int64_t i = 0;
    int64_t prev = 0;
    record_key_type nkey = *(reinterpret_cast<const record_key_type*>(key));
    // kArraySize is 256-bit(32 bytes) simd width
    // divided by sizeof(record_key_type)
    static const int32_t kArraySize = 256 / 8 / sizeof(record_key_type);
    record_key_type target[kArraySize];
    for (int32_t j = 0; j < kArraySize; j++) {
        target[j] = nkey;
    }
    record_key_type cmp_buffer[kArraySize];
    int16_t valid_size = 0;
    __m256i simd_target = _mm256_lddqu_si256(
        reinterpret_cast<const __m256i*>(target));
    __m256i simd_buffer;
    if (unlikely(first_[left] == nkey)) return first_ + left;
    if (unlikely(first_[right] == nkey)) return first_ + right;
    while (likely(left <= right)) {
        base = size >> bs_level_;
        rem = size & bs_mid_count_;
        for (i = 0; i < bs_mid_count_; i++) {
            mid[i] = left + base * i + (i > rem ? rem : i) + base / 2;
            if (unlikely(mid[i] > right)) break;
            cmp_buffer[i] = first_[mid[i]].key;
        }
        valid_size = i;
        simd_buffer = _mm256_lddqu_si256(
            reinterpret_cast<const __m256i*>(cmp_buffer));
#if SORT_TABLE_KEY_SIZE == 4
    simd_buffer = _mm256_cmpgt_epi32(simd_buffer, simd_target);
    for (i = 0; i < valid_size; i++) {
        bool need_break = false;
        switch (i) {
            case 0L:
                if (_mm256_extract_epi32(simd_buffer, 0)) need_break = true;
                break;
            case 1L:
                if (_mm256_extract_epi32(simd_buffer, 1)) need_break = true;
                break;
            case 2L:
                if (_mm256_extract_epi32(simd_buffer, 2)) need_break = true;
                break;
            case 3L:
                if (_mm256_extract_epi32(simd_buffer, 3)) need_break = true;
                break;
            case 4L:
                if (_mm256_extract_epi32(simd_buffer, 4)) need_break = true;
                break;
            case 5L:
                if (_mm256_extract_epi32(simd_buffer, 5)) need_break = true;
                break;
            case 6L:
                if (_mm256_extract_epi32(simd_buffer, 6)) need_break = true;
                break;
            case 7L:
                if (_mm256_extract_epi32(simd_buffer, 7)) need_break = true;
                break;
        }
        if (need_break) break;
    }
#elif SORT_TABLE_KEY_SIZE == 8
    simd_buffer = _mm256_cmpgt_epi64(simd_buffer, simd_target);
    for (i = 0; i < valid_size; i++) {
        bool need_break = false;
        switch (i) {
            case 0L:
                if (_mm256_extract_epi64(simd_buffer, 0)) need_break = true;
                break;
            case 1L:
                if (_mm256_extract_epi64(simd_buffer, 1)) need_break = true;
                break;
            case 2L:
                if (_mm256_extract_epi64(simd_buffer, 2)) need_break = true;
                break;
            case 3L:
                if (_mm256_extract_epi64(simd_buffer, 3)) need_break = true;
                break;
        }
#endif
    if (need_break) break;
    }
        prev = i - 1;
        if (unlikely(prev >= 0 && first_[mid[prev]] == nkey)) {
            return first_ + mid[prev];
        }
        if (i >= bs_mid_count_) {
            left = mid[last_mid] + 1;
        } else if (i <= 0) {
            right = mid[0] - 1;
        } else {
            left = mid[i - 1] + 1;
            right = mid[i] - 1;
        }
        size = right - left + 1;
    }
    if (pos != nullptr) *pos = right;
    return nullptr;
}

// TODO(zhanguo):Record 是POD类型, memmove 会快很多
inline Record* SortTable::AllocAdd(int64_t pos, const void* key) {
    int64_t i = 0;
    int64_t old_size = header_->total_size * sizeof(Record)
        + sizeof(SortTableHeader);
    int64_t new_size = old_size * 2;
    if (mmap_pool_ == nullptr) return nullptr;
    SortTableHeader* old_header = header_;
    char* new_header_addr = reinterpret_cast<char*>(mmap_pool_->Alloc(new_size));
    SortTableHeader* new_header
        = reinterpret_cast<SortTableHeader*>(new_header_addr);
    if (new_header == nullptr) return nullptr;
    __sync_lock_test_and_set(&(new_header->total_size),
        (new_size - sizeof(SortTableHeader)) / sizeof(Record));
    __sync_lock_test_and_set(&(new_header->used_size),
        old_header->used_size + 1);
    Record* new_first = reinterpret_cast<Record*>(new_header + 1);
    for (i = 0; i <= pos; i++) {
        new_first[i] = first_[i];
    }
    record_key_type nkey = *(reinterpret_cast<const record_key_type*>(key));
    new_first[i++].SetKey(nkey);
    for (; i < old_header->used_size + 1; i++) {
        new_first[i] = first_[i - 1];
    }
    // __sync_lock_test_and_set(&first_, new_first);
    // __sync_lock_test_and_set(&header_, new_header);
    first_ = new_first;
    header_ = new_header;
    __sync_fetch_and_or(&(old_header->total_size), 0x8000000000000000);
    if (!mmap_pool_->Free(old_header, old_size)) {
        return nullptr;
    }
    return first_ + pos + 1;
}

Record* SortTable::ExpandAdd(int64_t pos, const void* key) {
    int64_t expand_size =  header_->total_size * sizeof(Record);
    if (mmap_pool_ == nullptr) return nullptr;
    // 扩容一倍
    mmap_pool_->Alloc(expand_size);
    LOG_DEBUG("Expand sorttable form %ld to %ld, byte:%ld",
        header_->total_size, header_->total_size*2, expand_size);
    __sync_lock_test_and_set(&(header_->total_size),
        header_->total_size * 2);
    return DirectlyAdd(pos, key);
}

// TODO(zhanguo):Record 是POD类型, memmove 会快很多
inline Record* SortTable::DirectlyAdd(int64_t pos, const void* key) {
    for (int64_t i = header_->used_size - 1;
         i > pos; i--) {
        first_[i + 1] = first_[i];
    }
    record_key_type nkey = *(reinterpret_cast<const record_key_type*>(key));
    first_[pos + 1].SetKey(nkey);
    __sync_fetch_and_add(&(header_->used_size), 1);
    return first_ + pos + 1;
}

inline int64_t SortTable::GetHashKey(const void* key, int32_t len) {
    int64_t local_key = 0UL;
    if (len <= static_cast<int32_t>(sizeof(record_key_type))) {
        memcpy(&local_key, key, len);
    } else {
#if SORT_TABLE_KEY_SIZE == 4
        uint32_t high = Utils::MurmurHash2(key, len, kHighHashSeed);
        local_key = high;
#elif SORT_TABLE_KEY_SIZE == 8
        local_key = Utils::MurmurHash2(key, len, kHighHashSeed);
        local_key <<= 32;
        local_key |= Utils::MurmurHash2(key, len, kLowHashSeed);
#endif
    }
    return local_key;
}

Record* SortTable::GetRecordByKey(const void* key, int32_t len,
    bool allow_create, int64_t* ppos) {
    if (unlikely(key == nullptr)) {
        return nullptr;
    }
    int64_t local_key = GetHashKey(key, len);
    int64_t pos = -1L;
    Record* ret = nullptr;
    if (unlikely(bs_level_ < kMaxBinarySearchLevel)) {
        ret = NormalBinarySearch(&local_key, sizeof(record_key_type), &pos);
    } else {
        ret = SimdBinarySearch(&local_key, sizeof(record_key_type), &pos);
    }
    if (ret == nullptr && allow_create) {
        if (header_->total_size > header_->used_size) {
            ret = DirectlyAdd(pos, &local_key);
        } else if (type_ == SortTableType::EXCLUSIVE) {
            ret = ExpandAdd(pos, &local_key); // 原地扩容
        } else {
            ret = AllocAdd(pos, &local_key);
        }
    }
    if (ppos != nullptr) *ppos = pos;
    return ret;
}

bool SortTable::Insert(const KvPair& kv) {
    if (!kv.payload.Empty()) {
        LOG_DEBUG("sort table not support payload return false");
        return false;
    }
    Record* record = GetRecordByKey(kv.key, kv.len, true);
    if (record == nullptr) {
        return false;
    }
    (*record).SetValue(kv.value);
    return true;
}

bool SortTable::Delete(const KvPair& kv) {
  Record* record = GetRecordByKey(kv.key, kv.len);
  if (record == nullptr) {
    for (Record* r = record;
      r < first_ + header_->used_size - 1; r++) {
      *r = *(r + 1);
    }
    __sync_fetch_and_add(&header_->used_size, -1);
    return true;
  }
  return false;
}

bool SortTable::Search(KvPair* kv) {
  Record* record = GetRecordByKey(kv->key, kv->len);
  if (record == nullptr) {
    kv->value = -1L;
    return false;
  }
  kv->value = record->value;
  return true;
}

bool SortTable::BatchInsert(const std::vector<KvPair>& kvs,  bool already_sorted) {
    bool success = true;
    std::vector<KvPair> local_kvs(kvs.begin(), kvs.end());
    if (!already_sorted) {
        std::sort(local_kvs.begin(), local_kvs.end(),
            [this](const KvPair& left, const KvPair& right)->bool {
                return GetHashKey(left.key, left.len) <
                    GetHashKey(right.key, right.len);
            });
    }

    std::vector<int64_t> kv_index;
    std::vector<int64_t> positions;
    kv_index.reserve(local_kvs.size());
    positions.reserve(local_kvs.size());
    for (size_t i = 0; i < local_kvs.size(); i++) {
        const KvPair& kv = local_kvs[i];
        int64_t pos = -1L;
        Record* r = GetRecordByKey(kv.key, kv.len, false, &pos);
        if (r == nullptr) {
            kv_index.emplace_back(i);
            positions.emplace_back(pos);
        } else {
            // 处理已存在的键值对（此处省略具体逻辑）
        }
    }

    int32_t insert_count = positions.size();
    int64_t insert_index = positions.size() - 1;
    int64_t r = header_->used_size - 1;
    int64_t w = header_->used_size + insert_count - 1;
    if (header_->total_size >= header_->used_size + insert_count) {
        while (insert_index >= 0) {
            if (r >= 0 && r > positions[insert_index]) {
                *(first_ + w) = *(first_ + r);
                r--;
            } else {
                Record* record = first_ + w;
                record->key = GetHashKey(local_kvs[kv_index[insert_index]].key,
                    local_kvs[kv_index[insert_index]].len);
                record->value = local_kvs[kv_index[insert_index]].value;
                insert_index--;
            }
            w--;
        }
        __sync_fetch_and_add(&header_->used_size, insert_count);
    } else if (type_ == SortTableType::EXCLUSIVE) {
        // 预分配一些空间，至多多分配20% 或者4096个
        int64_t expect_size = header_->used_size + insert_count;
        int64_t new_size = std::max(expect_size * 1.2, (double)(expect_size + 4096));
        new_size = std::max(new_size, header_->total_size * 2);
        // 扩容逻辑（此处省略具体实现）
        // ...
    } else {
        // 共享模式下的扩容逻辑（此处省略具体实现）
        success = false;
    }
    return success;
}

bool SortTable::BatchDelete(const std::vector<KvPair>& kvs,
    bool already_sorted) {
    std::vector<KvPair> local_kvs(kvs.begin(), kvs.end());
    if (!already_sorted) {
        std::sort(local_kvs.begin(), local_kvs.end(),
            [this](const KvPair& left, const KvPair& right)->bool {
                return GetHashKey(left.key, left.len) <
                    GetHashKey(right.key, right.len);
            });
    }

    int64_t delete_count = 0;
    int64_t read_pos = 0;
    int64_t write_pos = 0;
    int64_t kv_index = 0;
    const int64_t total = header_->used_size;
    const int64_t kv_size = local_kvs.size();

    while (read_pos < total) {
        if (kv_index < kv_size) {
            record_key_type current_key = first_[read_pos].key;
            record_key_type target_key = GetHashKey(local_kvs[kv_index].key,
                local_kvs[kv_index].len);
            if (current_key == target_key) {
                // 找到要删除的记录，跳过不写
                kv_index++;
                delete_count++;
                read_pos++;
                continue;
            } else if (current_key < target_key) {
                // 当前记录不需要删除，写入新位置
                if (write_pos != read_pos) {
                    first_[write_pos] = first_[read_pos];
                }
                write_pos++;
                read_pos++;
            } else {
                // 目标键已被跳过，说明不存在
                kv_index++;
            }
        } else {
            // 剩余记录不需要删除，直接移动
            if (write_pos != read_pos) {
                first_[write_pos] = first_[read_pos];
            }
            write_pos++;
            read_pos++;
        }
    }

    header_->used_size -= delete_count;
    return true;
}



void SortTable::PrintTable() {
    LOG_DEBUG("SortTable: used_size=%ld, total_size=%ld",
        header_->used_size, header_->total_size);
    for (int64_t i = 0; i < header_->used_size; i++) {
        LOG_DEBUG("Record %ld: key=%ld, value=%ld",
            i, first_[i].key, first_[i].value);
    }
}

int64_t SortTable::GetUsedSize(int64_t offset) {
    // 假设offset是指向某个SortTableHeader的偏移量
    SortTableHeader* target_header = reinterpret_cast<SortTableHeader*>(
        mmap_pool_->GetMMapDataBegin() + offset);
    return target_header ? target_header->used_size : 0;
}

int64_t SortTable::GetTotalSize(int64_t offset) {
    SortTableHeader* target_header = reinterpret_cast<SortTableHeader*>(
        mmap_pool_->GetMMapDataBegin() + offset);
    return target_header ? target_header->total_size : 0;
}

// int64_t SortTable::GetHashKey(const void* key, int32_t len) {
//     if (key == nullptr || len <= 0) {
//         return 0;
//     }
//     // 使用MurmurHash计算键的哈希值
//     uint32_t hash = Utils::MurmurHash2(key, len, kHighHashSeed);
//     return static_cast<int64_t>(hash);
// }

// Record* SortTable::DirectlyAdd(int64_t pos, const void* key) {
//     // 在已有空间中直接插入记录
//     int64_t insert_pos = pos + 1;
//     // 移动后续记录
//     for (int64_t i = header_->used_size; i > insert_pos; i--) {
//         first_[i] = first_[i - 1];
//     }
//     // 插入新记录
//     first_[insert_pos].key = *(reinterpret_cast<const record_key_type*>(key));
//     first_[insert_pos].value = 0; // 默认值
//     header_->used_size++;
//     return first_ + insert_pos;
// }

// Record* SortTable::AllocAdd(int64_t pos, const void* key) {
//     // 分配新空间并插入记录（共享模式下使用）
//     // 此处省略具体实现
//     return nullptr;
// }

// Record* SortTable::ExpandAdd(int64_t pos, const void* key) {
//     // 扩容并插入记录（独占模式下使用）
//     int64_t new_size = header_->total_size * 2; // 翻倍扩容
//     // 分配新空间
//     void* new_addr = mmap_pool_->Alloc(sizeof(SortTableHeader) +
//         sizeof(Record) * new_size);
//     if (new_addr == nullptr) {
//         LOG_ERROR("ExpandAdd: alloc new space failed");
//         return nullptr;
//     }
//     // 复制旧数据
//     SortTableHeader* new_header = reinterpret_cast<SortTableHeader*>(new_addr);
//     new_header->used_size = header_->used_size;
//     new_header->total_size = new_size;
//     Record* new_first = reinterpret_cast<Record*>(new_header + 1);
//     memcpy(new_first, first_, sizeof(Record) * header_->used_size);
//     // 替换旧指针
//     header_ = new_header;
//     first_ = new_first;
//     // 插入新记录
//     return DirectlyAdd(pos, key);
// }


} // namespace index
} // namespace kernel