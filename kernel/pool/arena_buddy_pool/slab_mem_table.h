/*
Copyright [2021-3-2] <yingdawei>
*/
#pragma once
#include "mbase/log.h"
#include "mbase/data.h"
#include "kernel/pool/arena_buddy_pool/ab_pool_define.h"
#ifndef WAIMAI_AD_INDEX_KERNEL_SLAB_MEM_TABLE_H
#define WAIMAI_AD_INDEX_KERNEL_SLAB_MEM_TABLE_H

namespace kernel {
namespace pool {

struct SlabMemTableHead {
    uint64_t begin;
    uint64_t end;
    uint64_t current;
    uint32_t item_size;
    uint32_t use_item;
    uint32_t max_item_num;
};

struct SlabMemTable {
    SlabMemTableHead head;

    void Init(uint64_t addr, size_t size) {
        head.begin = addr;
        head.end = head.begin + SIZE_LIMIT_SMALL;
        head.item_size = size;
        head.current = head.begin + sizeof(SlabMemTableHead);
        head.max_item_num = (SIZE_LIMIT_SMALL - sizeof(SlabMemTableHead)) /
            head.item_size;
        head.use_item = 0;
    }

    uint64_t AllocData() {
        return AllocBlock();
    }

    uint64_t AllocBlock() {
        if (head.use_item < head.max_item_num) {
            uint64_t ret = head.begin + sizeof(SlabMemTableHead)
                + head.item_size * head.use_item;
            head.use_item++;
            return ret;
        }
        return ERROR_ALLOC_OFFSET;
    }
};

} // namespace pool
} // namespace kernel
#endif // WAIMAI_AD_INDEX_KERNEL_SLAB_MEM_TABLE_H