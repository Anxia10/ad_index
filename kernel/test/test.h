#pragma once
#include <iostream>
#include "tinyxml.h"

class Test {
    public:
        Test();
        ~Test();
        void PrintTest() {
            std::cout << 234 << std::endl;
        }
        void InitParament();
    protected:
        int parament;
};