#pragma once
#include <vector>
#include <memory>
#include "kernel/common/log/log.h"
#include "kernel/pool/mmap_pool.h"

namespace kernel {
namespace index {

struct KvPair {
    KvPair (const void* k = nullptr, int32_t l = 0, int64_t v = 0) :
        key(k), len(l), value(v) {
    };
    const void* key;
    int32_t len;
    int64_t value;
    data::Data payload;
    KvPair(const KvPair& kv) {
        key = kv.key;
        len = kv.len;
        value = kv.value;

    }
    bool operator<(const KvPair& kv) const {
        if (len == kv.len) {
            const char* l = reinterpret_cast<const char*>(key);
            const char* r = reinterpret_cast<const char*>(kv.key);
            for (int32_t i = 0; i < len; i++, l++, r++) {
                if (*l == *r) continue;
                return *l < *r;
            }
        } else {
            return len < kv.len;
        }
        return false;
    }
    void operator=(const KvPair& kv) {
        key = kv.key;
        len = kv.len;
        value = kv.value;
        payload = kv.payload;
    }
    KvPair& SetKey(const void* k) {
        key = k;
        return *this;
    }
    KvPair& SetLen(int32_t l) {
        len = l;
        return *this;
    }
    KvPair& SetValue(int64_t v) {
        value = v;
        return *this;
    }
    KvPair& SetPayload(const data::Data& p) {
        payload = p;
        return *this;
    }
};

struct IndexHeader {
    static constexpr int32_t kMagicLen = 16;
    static uint32_t kMagicVec[kMagicLen];
    bool CheckMagic() const {
        for (int32_t i = 0; i < kMagicLen; i++) {
            if (kMagicVec[i] != magic[i]) {
                return false;
            }
        }
        return true;
    }
    void Init() {
        for (int32_t i = 0; i < kMagicLen; i++) {
            magic[i] = kMagicVec[i];
        }
    }
    uint32_t magic[kMagicLen];
};

class Index {
    public:
        class Iterator {
            public:
                struct Element {
                    explicit Element(void* m = nullptr, void* e = nullptr) : meta(m), ele(e) {}
                    bool operator==(const Element& element) const {
                        return meta == element.meta && ele == element.ele;
                    }
                    void* meta;
                    void* ele;
                };
                explicit Iterator(const Index* index, const Element& ele) : index_(index), element_(ele) {}
                Iterator(const Iterator& it) {
                    index_ = it.index_;
                    element_ = it.element_;
                }
                Iterator& operator=(const Iterator& it) {
                    index_ = it.index_;
                    element_ = it.element_;
                    return *this;
                }
                ~Iterator() {}
                int64_t operator*() const {
                    return index_->ConvertElementToDocId(element_);
                }
                Iterator& operator++() {
                    index_->GetNextElement(&element_);
                    return *this;
                }
                Iterator& operator++(int32_t n) {
                    index_->GetNextElement(&element_);
                    return *this;
                }
                Iterator operator+(int32_t n) const {
                    return Iterator(index_, index_->GetNextElement(element_, n));
                }
                bool operator==(const Iterator& it) const {
                    return index_ == it.index_ && element_ == it.element_;
                }
                bool operator!=(const Iterator& it) const {
                    return !((*this) == it);
                }
                Element& GetElement() {
                    return element_;
            }
        private:
            const Index* index_;
            Element element_;
        };
    public:
    static bool CheckMagic(pool::MMapPool* mmap_pool);
    static bool MakeHeader(pool::MMapPool* mmap_pool);
    static size_t GetIndexHeaderSize() {
        return sizeof(IndexHeader);
    }

    public:
        Index() : mmap_pool_(nullptr), payload_size_(0) {}
        virtual ~Index() {}
        virtual bool Init(pool::MMapPool* mmap_pool, const char* begin = nullptr, bool create = true) = 0;
        virtual void Release() {}
        virtual bool Insert(const KvPair& kv) = 0;
        virtual bool Delete(const KvPair& kv) = 0;
        virtual bool Search(KvPair* kv) = 0;
        virtual int64_t GetTableEntryOffset() = 0;
        virtual int64_t GetSize() const = 0;
        virtual bool BatchInsert(const std::vector<KvPair>& kvs, bool already_sorted = false);
        virtual bool BatchDelete(const std::vector<KvPair>& kvs, bool already_sorted = false);
        virtual bool BatchSearch(std::vector<KvPair>* kvs, bool already_sorted = false);
        void SetPayloadSize(int32_t payload_size) {
            payload_size_ = payload_size;
        }

        // For Iterator
        virtual Iterator Begin(int64_t offset = -1L) const {
            return Iterator(this, Iterator::Element());
        }
        virtual Iterator End() const {
            return Iterator(this, Iterator::Element());
        }
        virtual KvPair ConvertElementToKvPair(const Iterator::Element& ele) const {
            return KvPair();
        }
        virtual int64_t ConvertElementToDocId(const Iterator::Element& ele) const {
            return -1L;
        }
        virtual Iterator::Element GetNextElement(const Iterator::Element& ele, int32_t n = 1) const {
            return Iterator::Element();
        }
        virtual void GetNextElement(Iterator::Element* ele, int32_t n = 1) const {
            return;
        }

    protected:
        pool::MMapPool* mmap_pool_;
        int32_t payload_size_;
        LOG_DECLARE;
};

}
}