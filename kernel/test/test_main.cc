#include <iostream>
// #include <test.h>
#include "kernel/test/test.h"

Test::Test() {

}

Test::~Test() {

}

int main()
{
    std::cout << 123 << std::endl;
    Test test;
    test.PrintTest();
    return 0;
}