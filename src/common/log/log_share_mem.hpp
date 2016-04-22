
#ifndef _EQP_LOG_SHARE_MEM_HPP_
#define _EQP_LOG_SHARE_MEM_HPP_

#include "define.hpp"
#include "semaphore.hpp"
#include "ring_buffer.hpp"
#include "server_op.hpp"
#include <atomic>

#define EQP_LOG_SHARE_MEM_NAME "shm/eqp-log-queue"

class LogShareMem
{
public:
    static const uint32_t SIZE = MEGABYTES(1);

private:
    Semaphore           m_semaphore;
    SharedRingBuffer    m_ringBuffer;

private:
    static uint32_t size() { return SIZE - offsetof(LogShareMem, m_ringBuffer); }

public:
    // Consumer methods
    void init();
    void wait() { m_semaphore.wait(); }
    bool pop(SharedRingBuffer::Packet& packet) { return m_ringBuffer.pop(packet); }

    // Producer methods
    bool push(SharedRingBuffer::Packet& packet);
    bool push(ServerOp opcode, int sourceId, uint32_t len, const char* message); // len should not include null terminator
    void post(); // Informs the consumer that there is new content to read
};

#endif//_EQP_LOG_SHARE_MEM_HPP_
