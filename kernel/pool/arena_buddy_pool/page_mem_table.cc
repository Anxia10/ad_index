#include "kernel/pool/arena_buddy_pool/page_mem_table.h"

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace kernel {
namespace pool {

PageMemTable::PageMemTable() :
    head_(nullptr), begin_(nullptr) {
}

PageMemTable::~PageMemTable() {
}

int64_t PageMemTable::AllocData(const size_t& len) {
    alloc_num++;
    uint64_t ret = ERROR_ALLOC_OFFSET;
    switch (divide_size(len)) {
        case SMALL_OBJECT:
            ret = AllocBySlab(len);
            break;
        case LARGE_OBJECT:
            ret = AllocByPageLevel(len);
            break;
        default:
            break;
    }
    if (ret == ERROR_ALLOC_OFFSET) {
        SetValidFlag();
        return ERROR_ALLOC_ADDR;
    }
    return reinterpret_cast<int64_t>(GetAddr(ret));
}

uint64_t PageMemTable::AllocByPageLevel(const size_t& len) {
    uint64_t ret = ERROR_ALLOC_OFFSET;
    uint16_t level = find_level(len);
    // 1. alloc from level
    PageInfo* page_info = free_lists[level];
    if (page_info->page_node_addr != INVALID_MEM_ADDR) {
        ret = page_info->page_node_addr;
        uint64_t* vaddr = reinterpret_cast<uint64_t*>(
            GetAddr(page_info->page_node_addr));
        page_info->page_node_addr = *vaddr;
        *vaddr = INVALID_MEM_ADDR;
        return ret;
    }
    // 2. alloc from slab node
    if (slab_info->slab_node_addr != INVALID_MEM_ADDR) {
        SlabMemTable* slab_mem_table = reinterpret_cast<SlabMemTable*>(
            GetAddr(slab_info->slab_node_addr));
        ret = slab_mem_table->AllocData();
        if (ret != ERROR_ALLOC_OFFSET) {
            return ret;
        }
    }
    // 3. need extend
    ret = ExtendFromPageMem(slab_level);
    if (ret != ERROR_ALLOC_OFFSET) {
        return ret;
    }
    // 4. extend page mem
    if (ExtendPageMem()) {
        ret = ExtendFromPageMem(slab_level);
        return ret;
    }
    return ret;
}

uint64_t PageMemTable::ExtendFromPageMem(const uint16_t& slab_level) {
    // 1. alloc from page mem
    uint64_t addr = GetUintFromPageLevel();
    if (addr == ERROR_ALLOC_OFFSET) {
        return ERROR_ALLOC_OFFSET;
    }
    // 2. init slab_mem_table
    slab_num++;
    SlabMemTable* slab_mem_table =
        reinterpret_cast<SlabMemTable*>(GetAddr(addr));
    slab_mem_table->Init(addr, slab_level_to_size(slab_level));
    // 3. move to slab_mem_list_
    slab_mem_list_[slab_level]->Update(addr);
    // 4. alloc
    return slab_mem_table->AllocData();
}

uint64_t PageMemTable::GetUintFromPageLevel() {
    uint64_t ret = ERROR_ALLOC_OFFSET;
    // 1. alloc from page level 0
    PageInfo* page_info = free_lists[0];
    if (page_info->page_node_addr != INVALID_MEM_ADDR) {
        ret = page_info->page_node_addr;
        uint64_t* vaddr = reinterpret_cast<uint64_t*>(
            GetAddr(page_info->page_node_addr));
        if (*vaddr != INVALID_MEM_ADDR) {
            page_info->page_node_addr = *vaddr;
        } else {
            page_info->page_node_addr = INVALID_MEM_ADDR;
        }
        return ret;
    }
    // 2. split high page level
    return SplitFromHighLevel(0);
}

uint64_t PageMemTable::SplitFromHighLevel(const uint16_t& level) {
    uint64_t ret = ERROR_ALLOC_OFFSET;
    uint16_t i = level + 1;
    for (; i < PAGE_MAX_LEVELS; i++) {
        PageInfo* page_info = free_lists[i];
        if (page_info->page_node_addr != INVALID_MEM_ADDR) {
            ret = page_info->page_node_addr;

            uint16_t current_level = i;
            uint64_t end_offset = page_info->page_node_addr +
                page_level_to_size(current_level);
            // move mem to low level
            for (; current_level > level; current_level--) {
                PageInfo* tmp = free_lists[current_level - 1];
                end_offset -= page_level_to_size(current_level - 1);
                uint64_t* vaddr = reinterpret_cast<uint64_t*>(
                    GetAddr(end_offset));
                *vaddr = tmp->page_node_addr;
                tmp->page_node_addr = end_offset;
            }
            // move high level mem
            uint64_t* vaddr = reinterpret_cast<uint64_t*>(
                GetAddr(page_info->page_node_addr));
            page_info->page_node_addr = *vaddr;
            *vaddr = INVALID_MEM_ADDR;
            return ret;
        }
    }
    return ret;
}

bool PageMemTable::ExtendPageMem() {
    for (int i = 0; i < PAGE_MAX_LEVELS; i++) {
        size_t size = page_level_to_size(i);
        uint64_t addr = AllocMem(size);
        if (addr != ERROR_ALLOC_OFFSET) {
            PageInfo* tmp = free_lists[i];
            uint64_t* new_addr = reinterpret_cast<uint64_t*>(
                GetAddr(addr));
            *new_addr = tmp->page_node_addr;
            tmp->page_node_addr = addr;
        } else {
            return false;
        }
    }
    return true;
}

bool PageMemTable::FreeData(uint64_t addr, size_t size) {
    switch (divide_size(size)) {
        case SMALL_OBJECT:
            return FreeDataBySlab(addr, size);
        case LARGE_OBJECT:
            return FreeDataByPage(addr, size);
        default:
            break;
    }
    return true;
}

bool PageMemTable::FreeDataByPage(uint64_t addr, size_t size) {
    uint16_t level = find_level(size);
    /*
    // 1. back to free_list_
    */
    // 2. MergePageLevel
    MergePageLevel(addr, level);

    return true;
}

bool PageMemTable::MergePageLevel(uint64_t head, const uint16_t& level) {
    // 1. back to free_list_
    PageInfo* page_info = free_lists[level];
    uint64_t* vaddr = reinterpret_cast<uint64_t*>(GetAddr(head));
    *vaddr = page_info->page_node_addr;
    page_info->page_node_addr = head;

    // 2. merge and back to high level
    uint64_t last = head;
    uint64_t now = *(reinterpret_cast<uint64_t*>(GetAddr(last)));
    size_t level_size = page_level_to_size(level);
    if (now - level_size == head || head - level_size == now) {
        free_lists[level]->page_node_addr =
            *(reinterpret_cast<uint64_t*>(GetAddr(now)));
        return MergePageLevel((head < now ? head : now), level + 1);
    }

    while (now != INVALID_MEM_ADDR) {
        if (now - level_size == head || head - level_size == now) {
            uint64_t* tmp = reinterpret_cast<uint64_t*>(GetAddr(now));
            *(reinterpret_cast<uint64_t*>(GetAddr(last))) = *tmp;
            free_lists[level]->page_node_addr =
                *(reinterpret_cast<uint64_t*>(GetAddr(head)));
            return MergePageLevel((head < now ? head : now), level + 1);
        }

        last = now;
        uint64_t* tmp = reinterpret_cast<uint64_t*>(GetAddr(now));
        now = *tmp;
    }
    return true;
}

bool PageMemTable::FreeDataBySlab(uint64_t addr, size_t size) {
    uint16_t slab_level = find_slab_level(size);
    SlabInfo* slab_info = slab_mem_list_[slab_level];

    uint64_t* vaddr = reinterpret_cast<uint64_t*>(GetAddr(addr));
    *vaddr = slab_info->free_block;
    slab_info->free_block = addr;
    return true;
}

} // namespace pool
} // namespace kernel