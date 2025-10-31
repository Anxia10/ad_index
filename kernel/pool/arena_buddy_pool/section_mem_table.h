/*
Copyright [2021-3-2] <yingdawei>
*/
#pragma once
#include <vector>
#include "mbase/log.h"
#include "kernel/pool/arena_buddy_pool/ab_pool_define.h"
#ifndef WAIMAI_AD_INDEX_KERNEL_SECTION_MEM_TABLE_H
#define WAIMAI_AD_INDEX_KERNEL_SECTION_MEM_TABLE_H

namespace kernel {
namespace pool {

struct SectionMemTableHead {
    uint64_t begin;
    uint64_t end;
    uint64_t current;
    uint64_t next;
    bool valid_flag;
    void Init(uint64_t start, size_t size) {
        begin = start;
        end = start + size;
        current = start + sizeof(SectionMemTableHead);
        next = INVALID_MEM_ADDR;
        valid_flag = true;
    }
};

class SectionMemTable {
public:
    SectionMemTable();
    virtual ~SectionMemTable();

    bool Init(uint64_t addr, char* begin,
        size_t size = DEFAULT_SECTION_MEM_SIZE);
    bool InitTable(uint64_t addr, char* begin,
        size_t size = DEFAULT_SECTION_MEM_SIZE);
    int64_t AllocData(const size_t& len);

    bool BelongToTable(uint64_t offset, size_t size) {
        if (offset > head_->begin
            && offset < head_->end
            && offset + size < head_->end) {
            return true;
        }
        return false;
    }
    uint64_t GetBegin() {
        return head_->begin;
    }
    uint64_t GetNext() {
        return head_->next;
    }

private:
    inline char* GetAddr(uint64_t offset) {
        return begin_ + offset;
    }

private:
    SectionMemTableHead* head_;
    char* begin_;
};

} // namespace pool
} // namespace kernel
#endif // WAIMAI_AD_INDEX_KERNEL_SECTION_MEM_TABLE_H