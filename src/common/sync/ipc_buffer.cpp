
#include "ipc_buffer.hpp"

void IpcBuffer::init()
{
    m_masterRingBuffer.init(RING_BUFFER_SIZE);
    m_remoteRingBuffer.init(RING_BUFFER_SIZE);
}

void IpcRemote::init(const char* path)
{
    m_masterSemaphore.init();
    
    m_shareMem.open(path, sizeof(IpcBuffer));
    m_ipcBuffer = (IpcBuffer*)m_shareMem.getMemory();
}

bool IpcRemote::pop(SharedRingBuffer::Packet& in)
{
    return m_ipcBuffer->remote().pop(in);
}

void IpcRemote::push(ServerOp opcode, int sourceId, uint32_t len, const byte* data)
{
    if (m_ipcBuffer->master().push(opcode, sourceId, len, data))
    {
        m_masterSemaphore.trigger();
        return;
    }
    
    std::lock_guard<AtomicMutex> lock(m_outQueueMutex);
    m_outQueue.emplace_back(opcode, sourceId, len, data);
}

void IpcRemote::pushThreadSafe(ServerOp opcode, int sourceId, uint32_t len, const byte* data)
{
    std::lock_guard<AtomicMutex> lock(m_outQueueMutex);
    m_outQueue.emplace_back(opcode, sourceId, len, data);
}

void IpcRemote::processOutQueue()
{
    std::lock_guard<AtomicMutex> lock(m_outQueueMutex);
    
    uint32_t i = 0;
    
    while (i < m_outQueue.size())
    {
        SharedRingBuffer::Packet& p = m_outQueue[i];
        
        if (!m_ipcBuffer->master().push(p.opcode(), p.sourceId(), p.length(), p.data()))
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
