#pragma once
#include <string>
#include <vector>
#include "kernel/common/log/log.h"
namespace kernel {

class Utils {
    public:
        static uint32_t MurmurHash2(const void* key, int32_t len, uint32_t seed);
    private:
        Utils() {};
        ~Utils() {};
};

} // namespace kernel
