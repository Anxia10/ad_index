#include "kernel/index/index.h"
namespace kernel {
namespace index {
LOG_SETUP("kernel", Index);

uint32_t IndexHeader::kMagicVec[IndexHeader::kMagicLen] = {
    0xD5C5473F, 0x236434A6, 0xD898C43C, 0x7B34A9B4, 0x90E7F69D,
    0x0FEE88E1, 0x89741124, 0xF57AC10B, 0xD25E5371, 0x87C37615,
    0x15EF7111, 0x8974ABCD, 0xB59ACD6B, 0x423E9371, 0xD898C43C,
    0xB59ACD6B
};

bool Index::CheckMagic(pool::MMapPool* mmap_pool) {
    if (mmap_pool == nullptr) {
        return false;
    }
    if (mmap_pool->GetDataSize() < sizeof(IndexHeader)) {
        return false;
    }
    const IndexHeader* header = reinterpret_cast<const IndexHeader*>(mmap_pool->GetMMapDataBegin());
    return (header != nullptr) && header->CheckMagic();
}

bool Index::MakeHeader(pool::MMapPool* mmap_pool) {
    if (mmap_pool == nullptr) {
        return false;
    }
    if (mmap_pool->GetDataSize() < sizeof(IndexHeader)) {
        if (!mmap_pool->Alloc(sizeof(IndexHeader))) {
            return false;
        }
    }
    const IndexHeader* const_header = reinterpret_cast<const IndexHeader*>(mmap_pool->GetMMapDataBegin());
    IndexHeader* header = const_cast<IndexHeader*>(const_header);
    if (header == nullptr) {
        return false;
    }
    header->Init();
    return true;
}

bool Index::BatchInsert(const std::vector<KvPair>& kvs, bool already_sorted) {
    bool success = true;
    for (const KvPair& kv : kvs) {
        if (!Insert(kv)) {
            success = false;
        }
    }
    return success;
}

bool Index::BatchSearch(std::vector<KvPair>* kvs, bool already_sorted) {
    bool success = true;
    for (KvPair& kv : *kvs) {
        if (!Search(&kv)) {
            success = false;
        }
    }
    return success;
}

bool Index::BatchDelete(const std::vector<KvPair>& kvs, bool already_sorted) {
    bool success = true;
    for (const KvPair& kv : kvs) {
        if (!Delete(kv)) {
            success = false;
        }
    }
    return success;
}
}  // namespace index
}  // namespace kernel
