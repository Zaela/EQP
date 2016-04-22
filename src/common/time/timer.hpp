
#ifndef _EQP_TIMER_HPP_
#define _EQP_TIMER_HPP_

#include "define.hpp"
#include <functional>

class TimerPool;

class Timer
{
public:
    typedef std::function<void(Timer*)> Callback;

private:
    friend class TimerPool;

    TimerPool&  m_pool;
    uint32_t    m_poolIndex;
    uint32_t    m_periodMilliseconds;
    Callback    m_callback;
    union
    {
        void*   m_userdata;
        int     m_luaCallback;
    };
    
    static const uint32_t INVALID_INDEX = 0xFFFFFFFF;

public:
    Timer(TimerPool& pool);
    Timer(TimerPool& pool, uint32_t periodMilliseconds, Callback callback, void* data = nullptr, bool start = true);
    ~Timer();

    void init(uint32_t periodMilliseconds, Callback callback, void* data = nullptr, bool start = true);

    uint32_t    getPeriodMilliseconds() const { return m_periodMilliseconds; }
    void*       userdata() const { return m_userdata; }

    void stop();
    void start() { restart(); }
    void restart();
    // Delaying an inactive/stopped timer has no effect.
    void delay(uint32_t milliseconds);
    // Forces the timer to trigger on the next cycle, as if its trigger time was reached.
    // If the timer is currently inactive/stopped, it is automatically restarted first.
    void forceTriggerOnNextCycle();
    
    // Invokes the timer's callback immediately, independent of the timing of its normal trigger cycle.
    void invokeCallback() { if (m_callback) m_callback(this); }
};

#endif//_EQP_TIMER_HPP_
