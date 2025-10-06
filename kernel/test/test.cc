#include "kernel/test/test.h"
#include <iostream>
#include <immintrin.h>
#include <avxintrin.h>

Test::Test() {

}

Test::~Test() {
    
}

void InitParament() {
}

int main()
{
    // 创建一个包含 4 个 64 位整数的向量
    __m256i vec = _mm256_setr_epi64x(10, 20, 30, 40);
    
    // 提取各个位置的元素
    long long elem0 = _mm256_extract_epi64(vec, 0);  // 返回 10
    long long elem1 = _mm256_extract_epi64(vec, 1);  // 返回 20
    long long elem2 = _mm256_extract_epi64(vec, 2);  // 返回 30
    long long elem3 = _mm256_extract_epi64(vec, 3);  // 返回 40
    
    std::cout << "Element 0: " << elem0 << std::endl;
    std::cout << "Element 1: " << elem1 << std::endl;
    std::cout << "Element 2: " << elem2 << std::endl;
    std::cout << "Element 3: " << elem3 << std::endl;
    
    return 0;
}