
#include "timer.hpp"
#include "timer_pool.hpp"

Timer::Timer(TimerPool& pool)
: m_pool(pool),
  m_poolIndex(INVALID_INDEX),
  m_periodMilliseconds(0),
  m_userdata(nullptr)
{
    
}

Timer::Timer(TimerPool& pool, uint32_t periodMilliseconds, Timer::Callback callback, void* userdata, bool start)
: m_pool(pool),
  m_poolIndex(INVALID_INDEX),
  m_periodMilliseconds(periodMilliseconds),
  m_callback(callback),
  m_userdata(userdata)
{
    if (start)
        pool.startTimer(this);
}

Timer::~Timer()
{
    stop();
}

void Timer::init(uint32_t periodMilliseconds, Timer::Callback callback, void* userdata, bool start)
{
    m_periodMilliseconds    = periodMilliseconds;
    m_callback              = callback;
    m_userdata              = userdata;
    
    if (start)
        m_pool.startTimer(this);
}

void Timer::stop()
{
    uint32_t index = m_poolIndex;
    
    if (index == INVALID_INDEX)
        return;
    
    m_poolIndex = INVALID_INDEX;
    m_pool.markTimerAsDead(index);
}

void Timer::restart()
{
    if (m_poolIndex == INVALID_INDEX)
    {
        m_pool.startTimer(this);
        return;
    }
    
    m_pool.restartTimer(this);
}

void Timer::delay(uint32_t milliseconds)
{
    if (m_poolIndex != INVALID_INDEX)
        m_pool.delayTimer(this, milliseconds);
}

void Timer::forceTriggerOnNextCycle()
{
    if (m_poolIndex == INVALID_INDEX)
        m_pool.startTimer(this);
    
    m_pool.forceTimerTrigger(this);
}
