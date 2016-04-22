
#include "semaphore.hpp"

Semaphore::Semaphore()
{
    init();
}

Semaphore::~Semaphore()
{
    destroy();
}

void Semaphore::init()
{
#ifdef EQP_WINDOWS
    m_semaphore = CreateSemaphore(nullptr, 0, numeric_limits<long>::max(), nullptr);
    
    if (!m_semaphore)
#else
    if (sem_init(&m_semaphore, 1, 0))
#endif
        throw 1; //fixme
}

void Semaphore::destroy()
{
#ifdef EQP_WINDOWS
    if (CloseHandle(m_semaphore))
#else
    if (sem_destroy(&m_semaphore))
#endif
        throw 1; //fixme
}

void Semaphore::wait()
{
#ifdef EQP_WINDOWS
    if (WaitForSingleObject(m_semaphore, INFINITE))
#else
    if (sem_wait(&m_semaphore))
#endif
        throw 1; //fixme
}

bool Semaphore::try_wait()
{
#ifdef EQP_WINDOWS
    if (WaitForSingleObject(m_semaphore, 0))
        return false;
#else
    if (sem_trywait(&m_semaphore))
    {
        int err = errno;
        
        if (err == EAGAIN)
            return false;
        
        throw 1; //fixme
    }
#endif
    
    return true;
}

void Semaphore::trigger()
{
#ifdef EQP_WINDOWS
    if (ReleaseSemaphore(m_semaphore, 1, nullptr))
#else
    if (sem_post(&m_semaphore))
#endif
        throw 1; //fixme
}
