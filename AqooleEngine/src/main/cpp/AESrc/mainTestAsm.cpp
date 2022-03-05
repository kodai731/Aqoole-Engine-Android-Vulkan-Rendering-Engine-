#define _USE_MATH_DEFINES
#define _GLM_FORCE_RADIANS
#include "vulkanMatrix.hpp"
#include <x86intrin.h>
#include <cstdlib>
#include <time.h>
#include <iostream>

time_t start, end;
const uint32_t TIMES = 100000;

int main()
{
    glm::vec3 v(0.0f, 1.0f, 2.0f);
    glm::mat3 m(1.0f);
    //asm
    start = clock();
    for(uint32_t i = 0; i < TIMES; i++)
        v = VulkanMatrix::TestMult(m, v);
    end = clock();
    std::cout << "asm = " << (double)(end - start) / CLOCKS_PER_SEC << std::endl;
    //glm
    start = clock();
    for(uint32_t i = 0; i < TIMES; i++)
        v = m * v;
    end = clock();
    std::cout << "glm = " << (double)(end - start) / CLOCKS_PER_SEC << std::endl;
    return 0;
}