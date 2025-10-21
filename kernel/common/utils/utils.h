#pragma once
#include <string>
#include <vector>
#include "kernel/common/data/data.h"
namespace kernel {

class Utils {
public:
    static void Split(const data::Data& str, const std::string& sep,
        std::vector<data::Data>* vec, bool ignore_empty = true);
    static void Split(const std::string& str, const std::string& sep,
        std::vector<data::Data>* vec, bool ignore_empty = true);
    static void Split(const std::string& str, const std::string& sep,
        std::vector<std::string>* vec, bool ignore_empty = true);
    static std::string Strip(const std::string& str);
    // support 6 section
    static std::string Join(const std::string& join_with,
        const std::string& section0,
        const std::string& section1 = std::string(),
        const std::string& section2 = std::string(),
        const std::string& section3 = std::string(),
        const std::string& section4 = std::string(),
        const std::string& section5 = std::string());
    static uint32_t MurmurHash2(const void* key, int32_t len,
        uint32_t seed);

private:
    Utils() {}
    ~Utils() {}
};

} // namespace kernel
