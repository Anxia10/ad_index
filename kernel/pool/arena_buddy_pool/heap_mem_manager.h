/*
Copyright [2021-3-2] <yingdawei>
*/
#pragma once
#include <vector>
#include "common/log/log.h"
#include "kernel/pool/arena_buddy_pool/ab_pool_define.h"
#include "kernel/pool/mmap_pool.h"
#include "kernel/pool/arena_buddy_pool/section_mem_table.h"
#include "kernel/pool/arena_buddy_pool/page_mem_table.h"

#ifndef WAIMAI_AD_INDEX_KERNEL_HEAP_MEM_MANAGER_H
#define WAIMAI_AD_INDEX_KERNEL_HEAP_MEM_MANAGER_H

namespace kernel {
namespace pool {

struct HeapHead {
    uint64_t begin;
    uint64_t end;
    uint64_t page_mem_start;
    uint64_t page_mem_end;
    uint64_t section_mem_start;
    uint64_t section_mem_end;
    void Init(uint64_t start) {
        begin = start;
        end = start + sizeof(HeapHead);
        page_mem_start = INVALID_MEM_ADDR;
        page_mem_end = INVALID_MEM_ADDR;
        section_mem_start = INVALID_MEM_ADDR;
        section_mem_end = INVALID_MEM_ADDR;
    }
};

class AreanaBuddyPool;
class HeapMemManager {
public:
    HeapMemManager();
    virtual ~HeapMemManager();

    uint64_t Alloc(size_t size);
    int64_t AllocData(const size_t& len);
    int64_t AllocDataFromPage(const size_t& len);
    int64_t AllocDataFromSection(const size_t& len);
    bool Free(uint64_t addr, size_t size);
    bool FreeDataFromPage(uint64_t addr, size_t size);
    bool FreeDataFromSection(uint64_t addr, size_t size);

    bool Init(AreanaBuddyPool* pool, char* begin) {
        if (pool == nullptr || begin == nullptr) {
            return false;
        }
        if (pool_ == nullptr) {
            pool_ = pool;
        }
        begin_ = begin;

        heap_head_ = reinterpret_cast<HeapHead*>(begin_);
        if (heap_head_->end == 0L) {
            return InitMmap(pool, begin);
        }
        // 1. init page mem
        uint64_t page_offset = heap_head_->page_mem_start;
        while (page_offset != INVALID_MEM_ADDR) {
            PageMemTable* page_mem_table = new PageMemTable();
            page_mem_table->InitTable(page_offset, begin_);
            page_mem_list_.push_back(page_mem_table);

            page_offset = page_mem_table->GetNext();
        }
        // 2. init section mem
        uint64_t section_offset = heap_head_->section_mem_start;
        while (section_offset != INVALID_MEM_ADDR) {
            SectionMemTable* section_mem_table = new SectionMemTable();
            section_mem_table->InitTable(section_offset, begin_);
            section_mem_list_.push_back(section_mem_table);

            section_offset = section_mem_table->GetNext();
        }
        return true;
    }

    bool InitMmap(AreanaBuddyPool* pool, char* begin) {
        if (pool == nullptr) {
            return false;
        }
        if (pool_ == nullptr) {
            pool_ = pool;
        }
        begin_ = begin;
        uint64_t offset = Alloc(sizeof(HeapHead));
        heap_head_->Init(offset);

        PageMemTable* page_mem_table = new PageMemTable();
        page_mem_table->Init(Alloc(DEFAULT_PAGE_MEM_SIZE), begin_);
        page_mem_list_.push_back(page_mem_table);
        heap_head_->page_mem_start = page_mem_table->GetBegin();

        SectionMemTable* section_mem_table = new SectionMemTable();
        section_mem_table->Init(Alloc(DEFAULT_SECTION_MEM_SIZE), begin_);
        section_mem_list_.push_back(section_mem_table);
        heap_head_->section_mem_start = section_mem_table->GetBegin();
        return true;
    }

    char* GetAddr(uint64_t offset) {
        return begin_ + offset;
    }

private:
    PageMemTable* ExtendPageMem(size_t num);

    void UpdatePageOffset(uint64_t offset) {
        if (heap_head_->page_mem_end == INVALID_MEM_ADDR) {
            heap_head_->page_mem_end = offset;
            return;
        }
        PageMemTableHead* head = reinterpret_cast<PageMemTableHead*>(
            GetAddr(heap_head_->page_mem_end));
        head->next = offset;
    }
    void UpdateSectionOffset(uint64_t offset) {
        if (heap_head_->section_mem_end == INVALID_MEM_ADDR) {
            heap_head_->section_mem_end = offset;
            return;
        }
        SectionMemTableHead* head = reinterpret_cast<SectionMemTableHead*>(
            GetAddr(heap_head_->section_mem_end));
        head->next = offset;
    }
    void CalculateExtendNum(size_t num) {
        extend_num_ += num;
    }

private:
    HeapHead* heap_head_;
    char* begin_;
    std::vector<PageMemTable*> page_mem_list_;
    std::vector<SectionMemTable*> section_mem_list_;
    AreanaBuddyPool* pool_;

    uint64_t extend_num_ = 0;
    uint64_t alloc_num_ = 0;
};

} // namespace pool
} // namespace kernel
#endif