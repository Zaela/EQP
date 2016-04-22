
#ifndef _EQP_MUTEX_HPP_
#define _EQP_MUTEX_HPP_

#include "define.hpp"
#include <atomic>
#include <mutex>
#include <thread>

#ifndef EQP_WINDOWS
#include <pthread.h>
#endif

class AtomicMutex
{
private:
    std::atomic_flag m_flag;

public:
    AtomicMutex();
    
    void lock();
    void unlock();
    bool try_lock();
};

class AtomicRecursiveMutex
{
private:
    std::atomic_flag                m_flag;
    std::thread::native_handle_type m_thread;
    int                             m_recurseCount;

private:
    void _lock()
    {
        m_thread = _thread();
        m_recurseCount++;
    }
    
    std::thread::native_handle_type _thread()
    {
    #ifdef EQP_WINDOWS
        return ::GetCurrentThread();
    #else
        return ::pthread_self();
    #endif
    }

public:
    AtomicRecursiveMutex();
    
    void lock();
    void unlock();
    bool try_lock();
};

#endif//_EQP_MUTEX_HPP_
