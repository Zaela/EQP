
#ifndef _EQP_SEMAPHORE_HPP_
#define _EQP_SEMAPHORE_HPP_

#include "define.hpp"

#ifdef EQP_LINUX
# include <semaphore.h>
#endif

class Semaphore
{
private:
#ifdef EQP_WINDOWS
    HANDLE  m_semaphore;
#else
    sem_t   m_semaphore;
#endif

public:
    Semaphore();
    ~Semaphore();

    void init();
    void destroy();

    void wait();
    bool try_wait();
    void trigger();
};

#endif//_EQP_SEMAPHORE_HPP_
