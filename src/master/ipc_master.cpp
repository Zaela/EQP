
#include "ipc_master.hpp"

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
    
    std::lock_guard<AtomicMutex> lock(m_outQueueMutex);
    m_outQueue.emplace_back(opcode, sourceId, len, (const byte*)data);
}

void IpcMaster::pushThreadSafe(ServerOp opcode, int sourceId, uint32_t len, const void* data)
{
    std::lock_guard<AtomicMutex> lock(m_outQueueMutex);
    m_outQueue.emplace_back(opcode, sourceId, len, (const byte*)data);
}

void IpcMaster::processOutQueue()
{
    std::lock_guard<AtomicMutex> lock(m_outQueueMutex);
    
    if (m_outQueue.empty())
        return;
    
    uint32_t i = 0;
    
    while (i < m_outQueue.size())
    {
        IpcPacket& p = m_outQueue[i];
        
        if (!m_ipcBuffer->remote().push(p.opcode(), p.sourceId(), p.length(), p.data()))
            break;
        
        i++;
    }
    
    if (i == m_outQueue.size())
    {
        m_outQueue.clear();
    }
    else
    {
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
}

