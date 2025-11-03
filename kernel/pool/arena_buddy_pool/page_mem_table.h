#pragma once
#include <math.h>
#include <vector>
#include "common/log/log.h"
#include "common/data/data.h"
#include "kernel/pool/arena_buddy_pool/slab_mem_table.h"
#include "kernel/pool/arena_buddy_pool/ab_pool_define.h"
#ifndef WAIMAI_AD_INDEX_KERNEL_PAGE_MEM_TABLE_H
#define WAIMAI_AD_INDEX_KERNEL_PAGE_MEM_TABLE_H

namespace kernel {
namespace pool {

struct PageMemTableHead {
    uint64_t begin;
    uint64_t end;
    uint64_t current;
    uint64_t next;
    bool valid_flag;
    int16_t fail_num;
    void Init(uint64_t start, size_t size) {
        begin = start;
        end = start + size;
        current = start;
        next = INVALID_MEM_ADDR;
        valid_flag = true;
        fail_num = 0;
    }
};

struct PageInfo {
    uint64_t page_node_addr;
};

struct SlabInfo {
    uint64_t slab_node_addr;
    uint64_t free_block;
    void Init(uint64_t start) {
        slab_node_addr = start;
        free_block = INVALID_MEM_ADDR;
    }
    void Update(uint64_t addr) {
        slab_node_addr = addr;
    }
};

class PageMemTable {
public:
    PageMemTable();
    virtual ~PageMemTable();

    int64_t AllocData(const size_t& len);
    uint64_t AllocByPageLevel(const size_t& len);
    uint64_t AllocBySlab(const size_t& len);

    bool ExtendPageMem();
    uint64_t ExtendFromPageMem(const uint16_t& slab_level);
    uint64_t SplitFromHighLevel(const uint16_t& slab_level);
    uint64_t GetUnitFromPageLevel();

    bool FreeData(uint64_t addr, size_t size);
    bool FreeDataByPage(uint64_t addr, size_t size);
    bool FreeDataBySlab(uint64_t addr, size_t size);

    bool MergePageLevel(uint64_t head, const uint16_t& level);

    bool Init(uint64_t addr, char* begin) {
        if (begin == nullptr) {
            return false;
        }
        begin_ = begin;
        head_ = reinterpret_cast<PageMemTableHead*>(begin + addr);
        head_->Init(addr, DEFAULT_PAGE_MEM_SIZE);

        AllocMem(sizeof(PageMemTableHead) +
            PAGE_MAX_LEVELS * sizeof(PageInfo) +
            SLAB_MAX_LEVELS * sizeof(SlabInfo));

        for (int i = 0; i < PAGE_MAX_LEVELS; i++) {
            size_t size = static_cast<size_t>(1024*pow(2.0, i + 2));
            uint64_t page_node_addr = AllocMem(size);
            free_lists_[i] = reinterpret_cast<PageInfo*>(GetPageInfoAddr(i));
            free_lists_[i]->page_node_addr = page_node_addr;
            *(reinterpret_cast<uint64_t*>(
                GetAddr(page_node_addr))) = INVALID_MEM_ADDR;
        }

        for (int j = 0; j < SLAB_MAX_LEVELS; j++) {
            SlabInfo* slab_info = reinterpret_cast<SlabInfo*>(GetSlabInfoAddr(j));
            slab_info->Init(INVALID_MEM_ADDR);
            slab_mem_list_[j] = slab_info;
        }
        return true;
    }

    bool InitTable(uint64_t addr, char* begin) {
        if (begin == nullptr) {
            return false;
        }
        begin_ = begin;
        head_ = reinterpret_cast<PageMemTableHead*>(begin + addr);

        for (int i = 0; i < PAGE_MAX_LEVELS; i++) {
            free_lists_[i] = reinterpret_cast<PageInfo*>(GetPageInfoAddr(i));
        }
        for (int j = 0; j < SLAB_MAX_LEVELS; j++) {
            slab_mem_list_[j] = reinterpret_cast<SlabInfo*>(GetSlabInfoAddr(j));
        }
        return true;
    }

    bool SetValidFlag(bool param = false) {
        if (head_->valid_flag) {
            head_->valid_flag = false;
        }
        return true;
    }

    bool GetValidFlag() {
        return head_->valid_flag;
    }

    uint64_t GetBegin() {
        return head_->begin;
    }

    uint64_t GetNext() {
        return head_->next;
    }

    bool BelongToTable(uint64_t offset, size_t size) {
        if (offset > head_->begin
            && offset < head_->end
            && offset + size < head_->end) {
            return true;
        }
        return false;
    }

private:
    inline char* GetAddr(uint64_t offset) {
        return begin_ + offset;
    }

    inline uint64_t AllocMem(size_t mem_len) {
        if (head_->current + mem_len > head_->end) {
            return ERROR_ALLOC_OFFSET;
        }
        uint64_t ret = head_->current;
        head_->current += mem_len;
        return ret;
    }

    inline char* GetPageInfoAddr(uint16_t level) {
        uint64_t offset = level * sizeof(PageInfo);
        return GetAddr(head_->begin + sizeof(PageMemTableHead) + offset);
    }

    inline char* GetSlabInfoAddr(uint16_t level) {
        uint64_t offset = level * sizeof(SlabInfo);
        return GetAddr(head_->begin + sizeof(PageMemTableHead) +
            PAGE_MAX_LEVELS * sizeof(PageInfo) + offset);
    }

    inline size_t page_level_to_size(uint16_t level) {
        double size = 1024*pow(2.0, level + 2);
        return static_cast<size_t>(size);
    }

    inline size_t slab_level_to_size(uint16_t level) {
        double size = 0.0;
        if (level < SLAB_MIN_LEVELS) {
            size = (level + 1) * SLAB_LEVEL_MIN_STEP;
        } else {
            size = SLAB_MIN_LEVELS * SLAB_LEVEL_MIN_STEP +
                (level - SLAB_MIN_LEVELS + 1) * SLAB_LEVEL_MAX_STEP;
        }
        return static_cast<size_t>(size);
    }

    inline uint16_t find_level(size_t size) {
        uint16_t k = 0;
        double n = 1024*pow(2.0, k + 2);
        while (k < PAGE_MAX_LEVELS
            && n < static_cast<double>(size)) {
            k++;
            n = 1024*pow(2.0, k + 2);
        }
        return (k < PAGE_MAX_LEVELS ? k : -1);
    }

    inline uint16_t find_slab_level(size_t size) {
        uint16_t k = 0;
        double n = slab_level_to_size(k);
        while (k < SLAB_MAX_LEVELS
            && n < static_cast<double>(size)) {
            k++;
            n = slab_level_to_size(k);
        }
        return (k < SLAB_MAX_LEVELS ? k : -1);
    }

    inline uint64_t get_page_node_tail(uint16_t level) {
        PageInfo* page_info = free_lists_[level];
        uint64_t vaddr = page_info->page_node_addr;
        if (vaddr == INVALID_MEM_ADDR) {
            return vaddr;
        }
        uint64_t* tmp = reinterpret_cast<uint64_t*>(GetAddr(vaddr));
        while (*tmp != INVALID_MEM_ADDR) {
            vaddr = *tmp;
            tmp = reinterpret_cast<uint64_t*>(GetAddr(vaddr));
        }
        return vaddr;
    }

    inline void free_lists_backword(const uint16_t& level) {
        PageInfo* page_info = free_lists_[level];
        uint64_t* vaddr = reinterpret_cast<uint64_t*>(
            GetAddr(page_info->page_node_addr));
        page_info->page_node_addr = *vaddr;
    }

private:
    PageMemTableHead* head_;
    PageInfo* free_lists_[PAGE_MAX_LEVELS];
    SlabInfo* slab_mem_list_[SLAB_MAX_LEVELS];
    char* begin_;

    uint64_t slab_num = 0;
    uint64_t alloc_num = 0;
};

} // namespace pool
} // namespace kernel
#endif // WAIMAI_AD_INDEX_KERNEL_PAGE_MEM_TABLE_H