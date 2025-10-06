#pragma once
#include "kernel/index/index.h"
#include "kernel/index/hash_table/hash_bucket.h"
#include "kernel/pool/mmap_pool.h"
#include "kernel/common/utils/utils.h"

namespace kernel {
namespace index {

struct MurMurHashTableHeader {
    static constexpr int32_t kMagicLen = 14;
    static uint32_t kMagicVec[kMagicLen];
    void Init(uint32_t bn) {
        bucket_num = bn;
        for (int32_t i = 0; i < kMagicLen; i++) {
            magic[i] = kMagicVec[i];
        }
        used_size = 0U;
    }
    bool CheckMagic() {
        for (int32_t i = 0; i < kMagicLen; i++) {
            if (magic[i] != kMagicVec[i]) return false;
        }
        return true;
    }
    uint32_t bucket_num;
    uint32_t used_size;
    uint32_t magic[kMagicLen];
};

class MurMurHashTable : public Index {
public:
    static constexpr uint32_t kRawKeyHashSeed = 0x89741511;
    static constexpr uint32_t kCheckKeyHashSeed = 0x91BA1109;
    static constexpr uint32_t kCheckPayloadHashSeed = 0x27EF9050;
    MurMurHashTable();
    ~MurMurHashTable();

    void SetBucketNum(uint32_t bucket_num);
    void Print();
    uint32_t GetBucketNum() {
        return bucket_num_;
    }

    bool Init(pool::MMapPool* mmap_pool, const char* begin = nullptr,
              bool create = true) override;
    bool Insert(const KvPair& kv) override;
    bool Delete(const KvPair& kv) override;
    bool Search(KvPair* kv) override;

    int64_t GetSize() const override {
        return header_ == nullptr ? 0UL : header_->used_size;
    }
    int64_t GetTableEntryOffset() override {
        return reinterpret_cast<char*>(header_) -
               mmap_pool_->GetMMapDataBegin();
    }

    Iterator Begin(int64_t offset = -1L) const override;
    int64_t ConvertElementToDocId(const Iterator::Element& ele) const override {
        HashBucket* r = reinterpret_cast<HashBucket*>(ele.ele);
        return r->GetValue();
    }
    Iterator::Element GetNextElement(const Iterator::Element& ele,
                                     int32_t n = 1) const override;
    void GetNextElement(Iterator::Element* ele, int32_t n = 1) const override;

private:
    bool InitEmptyHashTable(uint32_t bucket_num);
    bool LoadHashTable(const char* begin);
    HashBucket* GetBucketByKey(const void* key,
                               int32_t len, HashBucket** prev, bool allow_create = false);
    HashBucket* GetBucketByKeyLePayloadSize(
        const void* key, int32_t len, HashBucket** prev,
        bool allow_create = false);
    HashBucket* GetBucketByKeyGtPayloadSize(
        const void* key, int32_t len, HashBucket** prev,
        bool allow_create = false);
    HashBucket* GetRawBucketByKey(const void* key, int32_t len);

private:
    uint32_t bucket_num_;
    MurMurHashTableHeader* header_;
    HashBucket* first_;
    LOG_DECLARE;
};

}  // namespace index
}  // namespace kernel