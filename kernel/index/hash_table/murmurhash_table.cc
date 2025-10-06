#include "kernel/index/hash_table/murmurhash_table.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
namespace kernel {
namespace index {

LOG_SETUP("kernel", MurMurHashTable);

static const uint32_t kDefaultBucketNum = 256UL * 1024;
uint32_t MurMurHashTableHeader::kMagicVec[MurMurHashTableHeader::kMagicLen] = {
    0xD5C5473F, 0x236434A6, 0xD898C43C, 0x7B34A9B4, 0x90E7F69D,
    0x0FEE88E1, 0x89741124, 0xF57AC10B, 0xD25E5371, 0x87C37615,
    0x15EF7111, 0x8974ABCD, 0xB59ACD6B, 0x423E9372
};

MurMurHashTable::MurMurHashTable() :
    bucket_num_(kDefaultBucketNum),
    header_(nullptr), first_(nullptr) {
}

MurMurHashTable::~MurMurHashTable() {
}

bool MurMurHashTable::Init(pool::MMapPool* mmap_pool,
                           const char* begin, bool create) {
    if (mmap_pool == nullptr) {
        LOG_ERROR("MMapPool [%p] or begin [%p] is nullptr.",
                  mmap_pool, begin);
        return false;
    }
    mmap_pool_ = mmap_pool;
    if (!create) {
        if (!LoadHashTable(begin)) {
            LOG_ERROR("Load hashtable fail.");
            return false;
        }
    } else {
        if (!InitEmptyHashTable(bucket_num_)) {
            LOG_ERROR("Init empty hashtable fail.");
            return false;
        }
    }
    return true;
}

bool MurMurHashTable::LoadHashTable(const char* const_begin) {
    char* begin = const_cast<char*>(const_begin);
    header_ = reinterpret_cast<MurMurHashTableHeader*>(begin);
    if (header_ == nullptr) {
        LOG_ERROR("Begin addr is nullptr.");
        return false;
    }
    if (!header_->CheckMagic()) {
        LOG_ERROR("Hash table check magic fail. Maybe use incompatible version.");
        return false;
    }
    bucket_num_ = header_->bucket_num;
    first_ = reinterpret_cast<HashBucket*>(
        begin + sizeof(MurMurHashTableHeader));
    LOG_DEBUG("LoadHashTable success.");
    return true;
}

bool MurMurHashTable::InitEmptyHashTable(uint32_t bucket_num) {
    if (unlikely(mmap_pool_ == nullptr)) {
        LOG_ERROR("MMapPool is null.");
        return false;
    }
    void* addr = mmap_pool_->Alloc(sizeof(MurMurHashTableHeader)
                                  + sizeof(HashBucket) * bucket_num);
    header_ = reinterpret_cast<MurMurHashTableHeader*>(addr);
    if (unlikely(header_ == nullptr)) {
        LOG_ERROR("MMap alloc header fail.");
        return false;
    }
    header_->Init(bucket_num);

    first_ = reinterpret_cast<HashBucket*>(header_ + 1);
    if (unlikely(first_ == nullptr)) {
        LOG_ERROR("HashBucket alloc header fail.");
        return false;
    }
    for (uint32_t i = 0; i < bucket_num; i++) {
        first_[i].Clear();
    }
    return true;
}

void MurMurHashTable::SetBucketNum(uint32_t bucket_num) {
    bucket_num = bucket_num > 2 ? bucket_num : 2;
    uint32_t tmp = 1;
    for (int32_t i = 1; i < 32; i++) {
        tmp = (1 << i);
        if (tmp >= bucket_num) {
            break;
        }
    }
    bucket_num_ = tmp;
}

inline HashBucket* MurMurHashTable::GetRawBucketByKey(
    const void* key, int32_t len) {
    uint32_t hash = Utils::MurmurHash2(key, len, kRawKeyHashSeed);
    HashBucket* bucket = first_ + (hash & (bucket_num_ - 1));
    return bucket;
}

HashBucket* MurMurHashTable::GetBucketByKeyLePayloadSize(
    const void* key, int32_t len, HashBucket** prev,
    bool allow_create) {
    HashBucket* ret = nullptr;
    if (unlikely(key == nullptr || len == 0)) {
        return ret;
    }
    HashBucket* first_bucket = GetRawBucketByKey(key, len);
    HashBucket* bucket = first_bucket;
    bool first_bucket_available = false;
    if (unlikely(!bucket->BucketOccupied())) {
        first_bucket_available = true;
    } else if (likely(bucket->CheckKeyInPayload(key, len))) {
        *prev = nullptr;
        return bucket;
    }

    HashBucket* pre = nullptr;
    while (unlikely(bucket->GetNextBucket() >= bucket_num_)) {
        pre = bucket;
        bucket = first_ + bucket->GetNextBucket();
        if (likely(bucket->BucketOccupied() &&
                   bucket->CheckKeyInPayload(key, len))) {
            *prev = pre;
            return bucket;
        }
    }

    if (allow_create) {
        if (first_bucket_available) {
            ret = first_bucket;
            ret->SetKeyInPayload(key, len);
            *prev = nullptr;
        } else {
            if (unlikely(mmap_pool_ == nullptr)) {
                LOG_ERROR("MMapPool is null.");
                return ret;
            }
            ret = reinterpret_cast<HashBucket*>(
                mmap_pool_->Alloc(sizeof(HashBucket)));
            ret->SetKeyInPayload(key, len);
            bucket->SetNextBucket(ret - first_);
            *prev = bucket;
            ret->SetNextBucket(first_bucket - first_);
        }
    }
    return ret;
}

HashBucket* MurMurHashTable::GetBucketByKeyGtPayloadSize(
    const void* key, int32_t len, HashBucket** prev, bool allow_create) {
    if (unlikely(key == nullptr || len == 0)) {
        LOG_ERROR("Input invalid.");
        return nullptr;
    }
    HashBucket* first_bucket = GetRawBucketByKey(key, len);
    HashBucket* bucket = first_bucket;
    bool first_bucket_available = false;
    uint32_t check_code = Utils::MurmurHash2(key, len, kCheckKeyHashSeed);
    uint32_t payload_check_code = Utils::MurmurHash2(
        key, len, kCheckPayloadHashSeed);

    if (!bucket->BucketOccupied()) {
        first_bucket_available = true;
    } else if (bucket->GetKeySize() == len &&
               bucket->GetHashCode() == check_code &&
               bucket->CheckHashCodeInPayload(payload_check_code)) {
        *prev = nullptr;
        return bucket;
    }

    HashBucket* pre = nullptr;
    while (unlikely(bucket->GetNextBucket() >= bucket_num_)) {
        pre = bucket;
        bucket = first_ + bucket->GetNextBucket();
        if (likely(bucket->BucketOccupied() &&
                   bucket->GetKeySize() == len &&
                   bucket->GetHashCode() == check_code &&
                   bucket->CheckHashCodeInPayload(payload_check_code))) {
            *prev = pre;
            return bucket;
        }
    }

    if (allow_create) {
        HashBucket* ret = nullptr;
        if (first_bucket_available) {
            ret = first_bucket;
            ret->SetKeySize(len);
            ret->SetHashCode(check_code);
            ret->SetHashCodeInPayload(payload_check_code);
            *prev = nullptr;
        } else {
            if (unlikely(mmap_pool_ == nullptr)) {
                LOG_ERROR("MMapPool is null.");
                return ret;
            }
            ret = reinterpret_cast<HashBucket*>(
                mmap_pool_->Alloc(sizeof(HashBucket)));
            ret->SetKeySize(len);
            ret->SetHashCode(check_code);
            ret->SetHashCodeInPayload(payload_check_code);
            bucket->SetNextBucket(ret - first_);
            *prev = bucket;
            ret->SetNextBucket(first_bucket - first_);
        }
        return ret;
    }
    return nullptr;
}

HashBucket* MurMurHashTable::GetBucketByKey(
    const void* key, int32_t len, HashBucket** prev, bool allow_create) {
    return likely(len <= HASH_BUCKET_PAYLOAD_SIZE) ?
           GetBucketByKeyLePayloadSize(key, len, prev, allow_create) :
           GetBucketByKeyGtPayloadSize(key, len, prev, allow_create);
}

bool MurMurHashTable::Insert(const KvPair& kv) {
    if (!kv.payload.Empty()) {
        // murmurhash table not support payload
        LOG_ERROR("Murmurhash table not support payload");
        return false;
    }
    HashBucket* prev = nullptr;
    HashBucket* bucket = GetBucketByKey(kv.key, kv.len, &prev, true);
    if (unlikely(bucket == nullptr)) {
        LOG_ERROR("Get bucket fail.");
        return false;
    }
    if (!bucket->BucketOccupied()) {
        __sync_fetch_and_add(&(header_->used_size), 1);
    }
    bucket->SetValue(kv.value);
    bucket->MarkBucketOccupied();
    return true;
}

bool MurMurHashTable::Delete(const KvPair& kv) {
    HashBucket* prev = nullptr;
    HashBucket* bucket = GetBucketByKey(kv.key, kv.len, &prev);
    if (unlikely(bucket == nullptr)) {
        return true;
    }
    bucket->ClearBucketOccupied();
    __sync_fetch_and_sub(&(header_->used_size), 1);
    int64_t bucket_offset = bucket - first_;
    if (likely(bucket_offset >= 0L &&
               bucket_offset < bucket_num_)) {
        uint32_t next_bucket = bucket->GetNextBucket();
        if (next_bucket >= bucket_num_) {
            HashBucket* second = first_ + next_bucket;
            *bucket = *second;
            second->Clear();
            if (unlikely(mmap_pool_ == nullptr)) {
                LOG_ERROR("MMapPool is null.");
                return false;
            }
            if (!mmap_pool_->Free(second, sizeof(HashBucket))) {
                return false;
            }
        } else {
            uint32_t next_bucket = bucket->GetNextBucket();
            if (prev != nullptr) prev->SetNextBucket(next_bucket);
            bucket->Clear();
            if (unlikely(mmap_pool_ == nullptr)) {
                LOG_ERROR("MMapPool is null.");
                return false;
            }
            if (!mmap_pool_->Free(bucket, sizeof(HashBucket))) {
                return false;
            }
        }
    }
    return true;
}

bool MurMurHashTable::Search(KvPair* kv) {
    HashBucket* prev = nullptr;
    HashBucket* bucket = GetBucketByKey(kv->key, kv->len, &prev);
    if (unlikely(bucket == nullptr)) {
        kv->value = -1L;
        return false;
    }
    kv->value = bucket->GetValue();
    return true;
}

void MurMurHashTable::Print() {
    uint32_t next = 0U;
    for (uint32_t i = 0U; i < bucket_num_; i++) {
        HashBucket* bucket = first_ + i;
        do {
            next = bucket->GetNextBucket();
            printf("list[%u] bucket[%lu] hash[%u] next %u "
                   "value %ld occupied %d\n",
                   i, bucket - first_,
                   bucket->GetHashCode(), bucket->GetNextBucket(),
                   bucket->GetValue(), bucket->BucketOccupied());
            bucket = first_ + next;
        } while (next >= bucket_num_);
    }
}

Index::Iterator MurMurHashTable::Begin(int64_t offset) const {
    MurMurHashTableHeader* header = nullptr;
    if (offset == -1L || mmap_pool_ == nullptr) {
        header = header_;
    } else {
        header = reinterpret_cast<MurMurHashTableHeader*>(
            mmap_pool_->GetDataAddr(offset));
    }
    if (header == nullptr || header->used_size == 0U) {
        return Iterator(this, Iterator::Element());
    }
    HashBucket* bucket = first_;
    if (bucket->BucketOccupied()) {
        return Iterator(this, Iterator::Element(header, bucket));
    }

    // 此处代码可能不完整，根据图片内容呈现
    // ......

    return Iterator(this, Iterator::Element());
}

Index::Iterator::Element MurMurHashTable::GetNextElement(
    const Iterator::Element& ele, int32_t n) const {
    Iterator::Element ret = ele;
    GetNextElement(&ret, n);
    return ret;
}

void MurMurHashTable::GetNextElement(Index::Iterator::Element* ele,
                                     int32_t n) const {
    HashBucket* bucket = reinterpret_cast<HashBucket*>(ele->ele);
    if (bucket < first_ || n < 0) {
        ele->meta = nullptr;
        ele->ele = nullptr;
        return;
    }
    if (bucket->BucketOccupied()) n++;
    do {
        if (bucket->BucketOccupied() && --n <= 0) {
            ele->ele = bucket;
            return;
        }
        while (unlikely(bucket->GetNextBucket() >= bucket_num_)) {
            bucket = first_ + bucket->GetNextBucket();
            if (bucket->BucketOccupied() && --n <= 0) {
                ele->ele = bucket;
                return;
            }
        }
        while (bucket >= first_ + bucket_num_) {
            bucket = first_ + bucket->GetNextBucket();
        }
        bucket++;
    } while (bucket < first_ + bucket_num_);
    ele->meta = nullptr;
    ele->ele = nullptr;
}

}
}