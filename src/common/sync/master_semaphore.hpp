
#ifndef _EQP_MASTER_SEMAPHORE_HPP_
#define _EQP_MASTER_SEMAPHORE_HPP_

#include "define.hpp"
#include "semaphore.hpp"
#include "share_mem.hpp"

class MasterSemaphore
{
private:
    ShareMemViewer  m_shareMem;
    Semaphore*      m_semaphore;

public:
    void init()
    {
        m_shareMem.open("shm/master-semaphore", sizeof(Semaphore));
        m_semaphore = (Semaphore*)m_shareMem.getMemory();
    }
    
    void trigger() { m_semaphore->trigger(); }
};

#endif//_EQP_MASTER_SEMAPHORE_HPP_
