#include <string.h>

namespace kernel {
namespace pool {

#define DEFAULT_PAGE_MEM_SIZE 10*1024*1024
#define DEFAULT_SECTION_MEM_SIZE 1*1024*1024

#define DEFAULT_PAGE_MEM_TABLE_NUM 1
#define DEFAULT_SECTION_MEM_TABLE_NUM 1

#define PAGE_MAX_LEVELS 6
#define SLAB_MAX_LEVELS 46
#define SLAB_MIN_LEVELS 16
#define SLAB_LEVEL_MIN_STEP 8
#define SLAB_LEVEL_MAX_STEP 64

#define SIZE_LIMIT_SMALL 4*1024
#define SIZE_LIMIT_LARGE 128*1024

#define INVALID_MEM_ADDR 0xFFFFFFFF
#define ERROR_ALLOC_ADDR -1L
#define ERROR_ALLOC_OFFSET 0L

enum {
    SMALL_OBJECT = 0,
    LARGE_OBJECT = 1,
    HUGE_OBJECT = 2
};

static inline int32_t divide_size(size_t size) {
    if (size < SIZE_LIMIT_SMALL/2) {
        return SMALL_OBJECT;
    } else if (size < SIZE_LIMIT_LARGE) {
        return LARGE_OBJECT;
    } else {
        return HUGE_OBJECT;
    }
}

} // namespace pool
} // namespace kernel