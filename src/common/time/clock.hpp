
#ifndef _EQP_CLOCK_HPP_
#define _EQP_CLOCK_HPP_

#include <stdint.h>
#include <thread>
#include <chrono>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

class Clock
{
public:
    static uint64_t milliseconds();
    static uint64_t microseconds();
    static uint64_t unixSeconds();
    static void     sleepMilliseconds(uint32_t ms);
};

class PerfTimer
{
private:
    uint64_t m_microseconds;

public:
    PerfTimer() : m_microseconds(Clock::microseconds()) { }
    
    uint64_t microseconds() { return Clock::microseconds() - m_microseconds; }
};

#endif//_EQP_CLOCK_HPP_
