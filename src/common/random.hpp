
#ifndef _EQP_RANDOM_HPP_
#define _EQP_RANDOM_HPP_

#include "define.hpp"
#include <sqlite3.h>

class Random
{
public:
    static void     bytes(void* buffer, int count);
    static uint16_t uint16();
    static uint32_t uint32();
    static float    floatingPoint();
    static bool     chance(float percent);
};

#endif//_EQP_RANDOM_HPP_
