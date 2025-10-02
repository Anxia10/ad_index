#include "kernel/store/mmap_store.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static const size_t kDefaultMMapSize = 1UL * 1024 * 1024 * 1024;

namespace kernel {
namespace store {

MMapStore::MMapStore() :
    base_(nullptr), mmap_size_(kDefaultMMapSize),
    memory_lock_(false), memory_preload_(false){
}



}
}