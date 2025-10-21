#pragma once
#include <stdint.h>
#include <cstring>
namespace kernel {
namespace index {

#define HASH_BUCKET_PAYLOAD_SIZE 2
#define HASH_BUCKET_GEN_CODE(meaning, bitset) \
    bool meaning() { return flags_ & bitset; } \
    void Mark##meaning() { flags_ |= bitset; } \
    void Clear##meaning() { flags_ &= ~(bitset); }
static const size_t kCheckPayloadSize =
    static_cast<int64_t>(sizeof(uint32_t)) > HASH_BUCKET_PAYLOAD_SIZE ?
    HASH_BUCKET_PAYLOAD_SIZE : sizeof(uint32_t);
#pragma pack(push) // 内存对齐
#pragma pack(1)

class HashBucket {
public:
    HashBucket();
    ~HashBucket();
    HashBucket(const HashBucket& bucket) {
        key_size_ = bucket.key_size_;
        hash_code_ = bucket.hash_code_;
        value_ = bucket.value_;
        next_bucket_ = bucket.next_bucket_;
        memcpy(payload_, bucket.payload_, HASH_BUCKET_PAYLOAD_SIZE);
        // Flags copy is the last step. Ensure that.
        flags_ = bucket.flags_;
    }
    void operator=(const HashBucket& bucket) {
        key_size_ = bucket.key_size_;
        hash_code_ = bucket.hash_code_;
        value_ = bucket.value_;
        next_bucket_ = bucket.next_bucket_;
        memcpy(payload_, bucket.payload_, HASH_BUCKET_PAYLOAD_SIZE);
        // Flags copy is the last step. Ensure that.
        flags_ = bucket.flags_;
    }

    HASH_BUCKET_GEN_CODE(BucketOccupied, 0x1);
    HASH_BUCKET_GEN_CODE(HashCodeInPayload, 0x2);
    HASH_BUCKET_GEN_CODE(KeyInPayload, 0x4);

    void Clear() {
        memset(this, 0, sizeof(HashBucket));
    }
    void SetHashCodeInPayload(uint32_t hash_code) {
        memcpy(payload_, &hash_code, kCheckPayloadSize);
        MarkHashCodeInPayload();
    }
    bool CheckHashCodeInPayload(uint32_t hash_code) {
        if (!HashCodeInPayload()) {
            return false;
        }
        return memcmp(payload_, &hash_code, kCheckPayloadSize) == 0;
    }
    void SetKeyInPayload(const void* key, int32_t len) {
        if (key == nullptr) {
            return;
        }
        key_size_ = len;
        len = len > HASH_BUCKET_PAYLOAD_SIZE ? HASH_BUCKET_PAYLOAD_SIZE : len;
        memcpy(payload_, key, len);
        MarkKeyInPayload();
    }
    bool CheckKeyInPayload(const void* key, int32_t len);
    void SetNextBucket(uint32_t next) {
        next_bucket_ = next;
    }
    uint32_t GetNextBucket() {
        return next_bucket_;
    }
    void SetHashCode(uint32_t code) {
        hash_code_ = code;
    }
    uint32_t GetHashCode() {
        return hash_code_;
    }
    void SetValue(int64_t v) {
        value_ = v;
    }
    int64_t GetValue() {
        return value_;
    }
    void SetKeySize(uint8_t size) {
        key_size_ = size;
    }
    uint8_t GetKeySize() {
        return key_size_;
    }
private:
    int64_t value_ : 48;
    uint32_t next_bucket_ : 29;
    uint8_t flags_ : 3;
    uint32_t hash_code_;
    uint8_t key_size_;
    char payload_[HASH_BUCKET_PAYLOAD_SIZE];
};
#pragma pack(pop)

}  // namespace index
}  // namespace kernel