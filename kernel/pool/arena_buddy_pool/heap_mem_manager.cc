#include "kernel/pool/arena_buddy_pool/heap_mem_manager.h"
#include "kernel/pool/arena_buddy_pool/arena_buddy_pool.h"

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace pool {

HeapMemManager::HeapMemManager() :
    heap_head_(nullptr), pool_(nullptr) {
}

HeapMemManager::~HeapMemManager() {
}

uint64_t HeapMemManager::Alloc(size_t size) {
    void* addr = pool_->AllocNewStore(size);
    if (addr == nullptr) {
        return ERROR_ALLOC_OFFSET;
    }
    return reinterpret_cast<uint64_t>(addr) -
        reinterpret_cast<uint64_t>(begin_);
}

int64_t HeapMemManager::AllocData(const size_t& len) {
    int64_t ret = -1L;
    switch (divide_size(len)) {
        case SMALL_OBJECT:
        case LARGE_OBJECT:
            ret = AllocDataFromPage(len);
            break;
        case HUGE_OBJECT:
            ret = AllocDataFromSection(len);
            break;
        default:
            break;
    }
    return ret;
}

int64_t HeapMemManager::AllocDataFromPage(const size_t& len) {
    // 1. alloc from page mem list
    for (uint32_t i = 0; i < page_mem_list_.size(); i++) {
        if (page_mem_list_[i]->GetValidFlag()) {
            return page_mem_list_[i]->AllocData(len);
        }
    }
    // 2. need extend
    PageMemTable* page_mem_table = ExtendPageMem(1);
    if (page_mem_table != nullptr) {
        return page_mem_table->AllocData(len);
    } else {
        return -1L;
    }
}

int64_t HeapMemManager::AllocDataFromSection(const size_t& len) {
    int64_t ret = -1L;
    // 1. alloc from section mem list
    for (uint32_t i = 0; i < section_mem_list_.size(); i++) {
        ret = section_mem_list_[i]->AllocData(len);
        if (ret != ERROR_ALLOC_ADDR) {
            return ret;
        }
    }
    // 2. need extend
    SectionMemTable* section_mem_table = new SectionMemTable();
    section_mem_table->Init(Alloc(DEFAULT_SECTION_MEM_SIZE), begin_);
    section_mem_list_.push_back(section_mem_table);
    UpdateSectionOffset(section_mem_table->GetBegin());
    return section_mem_table->AllocData(len);
}

PageMemTable* HeapMemManager::ExtendPageMem(size_t num) {
    PageMemTable* ret = nullptr;
    for (uint32_t i = 0; i < num; i++) {
        PageMemTable* page_mem_table = new PageMemTable();
        page_mem_table->Init(Alloc(DEFAULT_PAGE_MEM_SIZE), begin_);
        page_mem_list_.push_back(page_mem_table);
        UpdatePageOffset(page_mem_table->GetBegin());
        if (i == 0) {
            ret = page_mem_table;
        }
    }
    CalculateExtendNum(num);
    return ret;
}

bool HeapMemManager::Free(uint64_t addr, size_t size) {
    if (addr == ERROR_ALLOC_OFFSET) {
        return true;
    }
    switch (divide_size(size)) {
        case SMALL_OBJECT:
        case LARGE_OBJECT:
            return FreeDataFromPage(addr, size);
        case HUGE_OBJECT:
            return FreeDataFromSection(addr, size);
        default:
            break;
    }
    return true;
}

bool HeapMemManager::FreeDataFromPage(uint64_t addr, size_t size) {
    // todo: need optimize
    for (size_t i = 0; i < page_mem_list_.size(); i++) {
        if (page_mem_list_[i]->BelongToTable(addr, size)) {
            return page_mem_list_[i]->FreeData(addr, size);
        }
    }
    return false;
}

bool HeapMemManager::FreeDataFromSection(uint64_t addr, size_t size) {
    return true;
}

} // namespace pool
} // namespace kernel