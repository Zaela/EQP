
#include "log_client.hpp"

LogClient::LogClient()
{
    m_shareMemCreator.create(EQP_LOG_SHARE_MEM_NAME, LogShareMem::SIZE);
    m_shareMemQueue = (LogShareMem*)m_shareMemCreator.getMemory();
    
    if (!m_shareMemQueue)
        throw 1; //fixme
    
    m_shareMemQueue->init();
}

LogClient::~LogClient()
{
    
}

void LogClient::push(int sourceId, const char* str, uint32_t len, ServerOp opcode)
{
    if (m_shareMemQueue->push(opcode, sourceId, len, str))
        return;
    
    m_pendingMessages.emplace_back(opcode, sourceId, len, (const byte*)str);
}

void LogClient::push(int sourceId, const std::string& str)
{
    push(sourceId, str.c_str(), str.length());
}

void LogClient::pushPending()
{
    uint32_t popped = 0;
    
    for (SharedRingBuffer::Packet& packet : m_pendingMessages)
    {
        if (!m_shareMemQueue->push(packet))
            break;
        
        popped++;
    }
    
    uint32_t size = m_pendingMessages.size();
    
    if (popped == size)
    {
        m_pendingMessages.clear();
        return;
    }
    
    if (popped == 0)
        return;
    
    size -= popped;
    
    for (uint32_t i = 0; i < size; i++)
        m_pendingMessages[i] = std::move(m_pendingMessages[i + popped]);
    
    for (uint32_t i = 0; i < popped; i++)
        m_pendingMessages.pop_back();
}

void LogClient::informServer()
{
    m_shareMemQueue->post();
}

void LogClient::sendExitSignalToServer()
{
    push(SourceId::Master, nullptr, 0, ServerOp::Shutdown);
    informServer();
}
