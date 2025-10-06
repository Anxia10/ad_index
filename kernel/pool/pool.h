#pragma once
#include <string>
#include <kernel/store/store.h>

namespace kernel {
namespace pool {

class Pool {
    public:
        Pool();
        virtual ~Pool();

        virtual bool Init(const std::string& file_name, bool read_only) = 0;
        virtual void* Alloc(size_t size) = 0;
        virtual bool Free(void* addr, size_t size) = 0;
        virtual bool Write(const data::Addr& addr, const data::Data& data) = 0;
        virtual bool Read(const data::Addr& addr, size_t len, data::Data* data) = 0;
        virtual int64_t NewData(const data::Data& data) = 0;
        virtual size_t GetPoolSize() = 0;
        virtual size_t GetUsedSize() = 0;
        virtual void Release() {};
        store::Store* GetStore() {
            return store_;
        }

    protected:
        store::Store* store_;
        bool read_only_;

    private:
        LOG_DECLARE;
};

}
}