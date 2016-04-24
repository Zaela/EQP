
#include "ipc_master.hpp"

IpcMaster::IpcMaster(MasterSemaphoreCreator& semaphore)
: m_semaphore(semaphore)
{
    
}

void IpcMaster::init(const char* path)
{
    m_shareMem.create(path, sizeof(IpcBuffer));
    m_ipcBuffer = (IpcBuffer*)m_shareMem.getMemory();
    
    m_ipcBuffer->init();
}

bool IpcMaster::pop(IpcPacket& in)
{
    return m_ipcBuffer->master().pop(in);
}

void IpcMaster::push(ServerOp opcode, int sourceId, uint32_t len, const void* data)
{
    if (m_ipcBuffer->remote().push(opcode, sourceId, len, (const byte*)data))
        return;
    
    m_outQueueMutex.lock();
    m_outQueue.emplace_back(opcode, sourceId, len, (const byte*)data);
    m_outQueueMutex.unlock();
    
    m_semaphore.trigger();
}

void IpcMaster::pushThreadSafe(ServerOp opcode, int sourceId, uint32_t len, const void* data)
{
    m_outQueueMutex.lock();
    m_outQueue.emplace_back(opcode, sourceId, len, (const byte*)data);
    m_outQueueMutex.unlock();
    
    m_semaphore.trigger();
}

void IpcMaster::forward(IpcPacket& packet, int sourceId)
{
    if (m_ipcBuffer->remote().push(packet.opcode(), sourceId, packet.length(), packet.data()))
        return;
    
    packet.setSourceId(sourceId);
    
    m_outQueueMutex.lock();
    m_outQueue.emplace_back(std::move(packet));
    m_outQueueMutex.unlock();
    
    m_semaphore.trigger();
}

void IpcMaster::processOutQueue()
{
    // Scope for lock_guard
    {
        std::lock_guard<AtomicMutex> lock(m_outQueueMutex);
        
        if (m_outQueue.empty())
            return;
        
        uint32_t i = 0;
        
        while (i < m_outQueue.size())
        {
            IpcPacket& p = m_outQueue[i];
            
            if (!m_ipcBuffer->remote().push(p.opcode(), p.sourceId(), p.length(), p.data()))
                goto incomplete;
            
            i++;
        }
        
        m_outQueue.clear();
        return;
        
    incomplete:
        
        uint32_t count = m_outQueue.size() - i;
        for (uint32_t j = 0; j < count; j++)
        {
            m_outQueue[j] = std::move(m_outQueue[j + i]);
        }
        
        for (uint32_t j = 0; j < count; j++)
        {
            m_outQueue.pop_back();
        }
    }
    
    // If we reached here, the out queue still has contents
    // Re-trigger the semaphore to ensure the IPC thread will keep trying
    m_semaphore.trigger();
}

