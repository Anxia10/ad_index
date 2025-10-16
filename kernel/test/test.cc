#include "kernel/test/test.h"
#include <iostream>
#include <immintrin.h>
#include <avxintrin.h>
#include<unordered_map>

Test::Test() {

}

Test::~Test() {
    
}

void InitParament() {
}

int main()
{
    // const char* cstr = "hello";
    // void* str = cstr; 

    std::string a = "1234";
    char* p = const_cast<char*>(a.data()) ;
    char* ch_p = reinterpret_cast<char*>(p);
    std::cout << ch_p[0] << std::endl;
    std::cout << ch_p[1] << std::endl;
    std::cout << ch_p << std::endl;
    
    return 0;
}