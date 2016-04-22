
#include "timer_pool.hpp"

TimerPool::TimerPool()
: m_capacity(DEFAULT_CAPACITY),
  m_count(0),
  m_triggerTimes(nullptr),
  m_timerObjects(nullptr)
{

}

TimerPool::~TimerPool()
{
    if (m_triggerTimes)
        delete[] m_triggerTimes;
    
    if (m_timerObjects)
        delete[] m_timerObjects;
}

void TimerPool::init()
{
    m_triggerTimes = new uint64_t[DEFAULT_CAPACITY];
    m_timerObjects = new Timer*[DEFAULT_CAPACITY];
}

void TimerPool::startTimer(Timer* timer)
{
    uint64_t curTime    = Clock::milliseconds();
    uint32_t index      = m_count++;

    if (index >= m_capacity)
        realloc();
    
    m_triggerTimes[index] = curTime + timer->getPeriodMilliseconds();
    m_timerObjects[index] = timer;
    
    timer->m_poolIndex = index;
}

void TimerPool::restartTimer(Timer* timer)
{
    m_triggerTimes[timer->m_poolIndex] = Clock::milliseconds() + timer->getPeriodMilliseconds();
}

void TimerPool::delayTimer(Timer* timer, uint32_t milliseconds)
{
    m_triggerTimes[timer->m_poolIndex] += milliseconds;
}

void TimerPool::forceTimerTrigger(Timer* timer)
{
    m_triggerTimes[timer->m_poolIndex] = Clock::milliseconds();
}

#define set_dead(val) val |= 1ULL << 63

void TimerPool::markTimerAsDead(uint32_t index)
{
    set_dead(m_triggerTimes[index]);
}

void TimerPool::swapAndPop(uint32_t index)
{
    uint32_t back = m_count - 1;
    
    if (back != index)
    {
        Timer* timer            = m_timerObjects[back];
        m_timerObjects[index]   = timer;
        m_triggerTimes[index]   = m_triggerTimes[back];
        timer->m_poolIndex      = index;
    }
    
    m_count = back;
}

void TimerPool::triggerTimer(uint32_t index)
{
    Timer* timer = m_timerObjects[index];
    
    m_triggerTimes[index] += timer->getPeriodMilliseconds();
    
    timer->invokeCallback();
}

void TimerPool::realloc()
{
    uint32_t c = m_capacity * 2;
    
    uint64_t* triggerTimes = new uint64_t[c];
    memcpy(triggerTimes, m_triggerTimes, m_capacity * sizeof(uint64_t));
    delete[] m_triggerTimes;
    m_triggerTimes = triggerTimes;
    
    Timer** timerObjects = new Timer*[c];
    memcpy(timerObjects, m_timerObjects, m_capacity * sizeof(Timer*));
    delete[] m_timerObjects;
    m_timerObjects = timerObjects;
    
    m_capacity = c;
}

#define is_dead(t) (get_bit64(t, 63))
#define check_time(t) (t & bitmask64(63))

void TimerPool::executeTimerCallbacks()
{
    uint32_t i              = 0;
    uint32_t n              = m_count;
    uint64_t* triggerTimes  = m_triggerTimes;
    uint64_t curTime        = Clock::milliseconds();
    
    // Guarantee: nothing in this loop will cause m_triggerTimes to relocate
    while (i < n)
    {
        uint64_t triggerTime = triggerTimes[i];
        
        if (is_dead(triggerTime))
        {
            swapAndPop(i);
            n--;
            continue;
        }
        
        if (curTime >= check_time(triggerTime))
            m_triggered.push_back(i);
        
        i++;
    }
    
    // Invoke callbacks. Safe for m_triggerTimes to be relocated as a result of these
    for (uint32_t index : m_triggered)
        triggerTimer(index);
    
    m_triggered.clear();
}

Timer* TimerPool::createTimer(uint32_t periodMilliseconds, Timer::Callback callback, void* userdata)
{
    return new Timer(*this, periodMilliseconds, callback, userdata);
}
