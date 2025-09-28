#pragma once
#include <cstdint>
#include<iostream>
#include "kernel/index/index.h"


namespace kernel {
namespace index {

struct MurMurHashTableHeader{

    uint32_t bucket_num;
    uint32_t userd_size;
};



class MurMurHashTable : public Index {
    public:
        MurMurHashTable();
        ~MurMurHashTable();
        bool Init() override;
    private:

};

} 
}