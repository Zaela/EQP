
#include "log_share_mem.hpp"

void LogShareMem::init()
{
    m_semaphore.init();
    m_ringBuffer.init(size());
}

bool LogShareMem::push(SharedRingBuffer::Packet& p)
{
    return m_ringBuffer.push(p.opcode(), p.sourceId(), p.length(), p.data());
}

bool LogShareMem::push(ServerOp opcode, int sourceId, uint32_t len, const char* message)
{
    return m_ringBuffer.push(opcode, sourceId, len, (const byte*)message);
}

void LogShareMem::post()
{
    m_semaphore.trigger();
}
