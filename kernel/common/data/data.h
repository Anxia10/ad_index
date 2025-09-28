#pragma once
#include<string>

namespace kernel {
namespace data {

struct Addr {
    explicit Addr(void* p = nullptr): 
        addr(p) {} 
    void* addr;
};

struct Data {
    explicit Data(const void* p = nullptr, size_t l = 0) :
        data(p),
        len(l){};
    
    bool Empty() const {
        return len == 0UL;
    }

    void Assign(const void* p, size_t l) {
        data = p;
        len = l;
    }

    void Assign(const Data& d) {
        data = d.data;
        len = d.len;
    }

    void Assign(const std::string& s) {
        data = s.c_str();
        len = s.size();
    }

    std::string ToString() const {
        return std::string(reinterpret_cast<const char*>(data) ,len);
    }

    void clear() {
        len = 0UL;
    }

    const void* data; //只读
    size_t len;
};



}
}