
#ifndef _EQP_LOG_CLIENT_HPP_
#define _EQP_LOG_CLIENT_HPP_

#include "define.hpp"
#include "share_mem.hpp"
#include "log_share_mem.hpp"
#include "server_op.hpp"
#include "source_id.hpp"
#include <string>
#include <vector>

class LogClient
{
private:
    LogShareMem*    m_shareMemQueue;
    ShareMemCreator m_shareMemCreator;

    std::vector<SharedRingBuffer::Packet> m_pendingMessages;

public:
    LogClient();
    ~LogClient();

    void push(int sourceId, const char* str, uint32_t len, ServerOp opcode = ServerOp::LogMessage);
    void push(int sourceId, const std::string& str);
    void pushPending();
    void informServer();
    void sendExitSignalToServer();

    bool hasPending() const { return !m_pendingMessages.empty(); }
};

#endif//_EQP_LOG_CLIENT_HPP_
