
#ifndef _EQP_MASTER_SEMAPHORE_CREATOR_HPP_
#define _EQP_MASTER_SEMAPHORE_CREATOR_HPP_

#include "define.hpp"
#include "semaphore.hpp"
#include "share_mem.hpp"

class MasterSemaphoreCreator
{
private:
    ShareMemCreator m_shareMem;
    Semaphore*      m_semaphore;

public:
    void init()
    {
        m_shareMem.create("shm/master-semaphore", sizeof(Semaphore));
        m_semaphore = (Semaphore*)m_shareMem.getMemory();
        
        m_semaphore->init();
    }
    
    void wait() { m_semaphore->wait(); }
};

#endif//_EQP_MASTER_SEMAPHORE_CREATOR_HPP_
