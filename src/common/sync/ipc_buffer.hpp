
#ifndef _EQP_IPC_BUFFER_HPP_
#define _EQP_IPC_BUFFER_HPP_

#include "define.hpp"
#include "ring_buffer.hpp"
#include "semaphore.hpp"
#include "atomic_mutex.hpp"
#include "share_mem.hpp"
#include "server_op.hpp"
#include "source_id.hpp"
#include "master_semaphore.hpp"
#include <vector>

#define EQP_IPC_LOGIN "shm/ipc-login"
#define EQP_IPC_CHAR_SELECT "shm/ipc-char-select"

// Shared Memory object
class IpcBuffer
{
private:
    static const uint32_t RING_BUFFER_SIZE = MEGABYTES(1) / 2;

private:
    union
    {
        SharedRingBuffer    m_masterRingBuffer;
        byte                m_masterBytes[RING_BUFFER_SIZE];
    };
    union
    {
        SharedRingBuffer    m_remoteRingBuffer;
        byte                m_remoteBytes[RING_BUFFER_SIZE];
    };
    
public:
    void init();
    
    SharedRingBuffer& master() { return m_masterRingBuffer; }
    SharedRingBuffer& remote() { return m_remoteRingBuffer; }
};

class IpcRemote
{
private:
    MasterSemaphore m_masterSemaphore;
    ShareMemViewer  m_shareMem;
    IpcBuffer*      m_ipcBuffer;

    AtomicMutex             m_outQueueMutex;
    std::vector<IpcPacket>  m_outQueue;

public:
    void init(const char* path);

    bool pop(IpcPacket& in);
    // Not thread safe; should only be called from the main thread. Use pushThreadSafe() otherwise.
    void push(ServerOp opcode, int sourceId, uint32_t len, const void* data);
    void pushThreadSafe(ServerOp opcode, int sourceId, uint32_t len, const void* data);

    void processOutQueue();
};

#endif//_EQP_IPC_BUFFER_HPP_
