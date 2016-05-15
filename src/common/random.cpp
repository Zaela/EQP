
#include "random.hpp"

void Random::bytes(void* buffer, int count)
{
    sqlite3_randomness(count, (byte*)buffer);
}

uint16_t Random::uint16()
{
    uint16_t ret;
    Random::bytes(&ret, sizeof(uint16_t));
    return ret;
}

uint32_t Random::uint32()
{
    uint32_t ret;
    Random::bytes(&ret, sizeof(uint32_t));
    return ret;
}

float Random::floatingPoint()
{
    float ret;
    Random::bytes(&ret, sizeof(float));
    return ret;
}

bool Random::chance(float percent)
{
    return fmod(floatingPoint(), 1.0) < percent;
}
