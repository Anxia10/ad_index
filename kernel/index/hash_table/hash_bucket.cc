#include <stdio.h>
#include <immintrin.h>
#include <avxintrin.h>
#include "kernel/index/hash_table/hash_bucket.h"
namespace kernel {
namespace index {

HashBucket::HashBucket() :
    value_(0L),
    next_bucket_(0U),
    flags_(0),
    hash_code_(0),
    key_size_(0) {
    memset(payload_, 0, HASH_BUCKET_PAYLOAD_SIZE);
}

HashBucket::~HashBucket() {
}

bool HashBucket::CheckKeyInPayload(const void* key, int32_t len) {
    if (key == nullptr || !KeyInPayload() || len != key_size_) {
        return false;
    }
    len = len > HASH_BUCKET_PAYLOAD_SIZE ? HASH_BUCKET_PAYLOAD_SIZE : len;
    const char* ptr = reinterpret_cast<const char*>(key);
    // 高效的校验内存数据是否一致
    int32_t index = 0;
    __m256i left;
    __m256i right;
    bool neq = true;
    while (len >= 32) {
        left = _mm256_lddqu_si256(reinterpret_cast<const __m256i*>(ptr + index));
        right = _mm256_lddqu_si256(reinterpret_cast<const __m256i*>(payload_ + index));
        left = _mm256_cmpeq_epi64(left, right);

        neq = 0L == _mm256_extract_epi64(left, 0) ||
              0L == _mm256_extract_epi64(left, 1) ||
              0L == _mm256_extract_epi64(left, 2) ||
              0L == _mm256_extract_epi64(left, 3);
        if (neq) {
            return false;
        }
        len -= 32;
        index += 32;
    }
    return memcmp(ptr + index, payload_ + index, len) == 0;
}

} 
}