
#include "atomic_mutex.hpp"

AtomicMutex::AtomicMutex()
: m_flag(ATOMIC_FLAG_INIT)
{
    
}

void AtomicMutex::lock()
{
    for (;;)
    {
        if (try_lock())
            return;
    }
}

void AtomicMutex::unlock()
{
    m_flag.clear();
}

bool AtomicMutex::try_lock()
{
    return m_flag.test_and_set() == false;
}

AtomicRecursiveMutex::AtomicRecursiveMutex()
: m_flag(ATOMIC_FLAG_INIT),
  m_thread(0),
  m_recurseCount(0)
{
    
}

void AtomicRecursiveMutex::lock()
{
    for (;;)
    {
        if (try_lock())
            return;
    }
}

void AtomicRecursiveMutex::unlock()
{
    if (m_thread != _thread())
        return;
    
    if (--m_recurseCount > 0)
        return;
    
    m_thread        = 0;
    m_recurseCount  = 0;
    m_flag.clear();
}

bool AtomicRecursiveMutex::try_lock()
{
    std::thread::native_handle_type t = _thread();
    
    if (m_thread == t || m_flag.test_and_set() == false)
    {
        m_thread = t;
        m_recurseCount++;
        return true;
    }
    
    return false;
}
