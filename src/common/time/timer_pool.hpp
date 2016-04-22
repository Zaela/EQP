
#ifndef _EQP_TIMER_POOL_HPP_
#define _EQP_TIMER_POOL_HPP_

#include "define.hpp"
#include "timer.hpp"
#include "clock.hpp"
#include "bit.hpp"
#include "atomic_mutex.hpp"
#include <vector>

class TimerPool
{
private:
    uint32_t                m_capacity;
    uint32_t                m_count;
    uint64_t*               m_triggerTimes;
    Timer**                 m_timerObjects;
    std::vector<uint32_t>   m_triggered;

    static const uint32_t DEFAULT_CAPACITY = 32;

private:
    friend class Timer;
    void swapAndPop(uint32_t index);
    void triggerTimer(uint32_t index);
    void realloc();

    void startTimer(Timer* timer);
    void restartTimer(Timer* timer);
    void delayTimer(Timer* timer, uint32_t milliseconds);
    void forceTimerTrigger(Timer* timer);
    void markTimerAsDead(uint32_t index);

public:
    TimerPool();
    ~TimerPool();

    void init();

    Timer* createTimer(uint32_t periodMilliseconds, Timer::Callback callback, void* userdata = nullptr);

    void executeTimerCallbacks();
};

#endif//_EQP_TIMER_POOL_HPP_
