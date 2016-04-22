
#ifndef _EQP_IPC_MASTER_HPP_
#define _EQP_IPC_MASTER_HPP_

#include "define.hpp"
#include "ring_buffer.hpp"
#include "ipc_buffer.hpp"
#include "share_mem.hpp"
#include "atomic_mutex.hpp"
#include <vector>

class IpcMaster
{
private:
    ShareMemCreator m_shareMem;
    IpcBuffer*      m_ipcBuffer;

    AtomicMutex                             m_outQueueMutex;
    std::vector<SharedRingBuffer::Packet>   m_outQueue;

public:
    void init(const char* path);

    bool pop(SharedRingBuffer::Packet& in);
    // Not thread safe; should only be called from the main thread. Use pushThreadSafe() otherwise.
    void push(ServerOp opcode, int sourceId, uint32_t len, const byte* data);
    void pushThreadSafe(ServerOp opcode, int sourceId, uint32_t len, const byte* data);

    void processOutQueue();
};

#endif//_EQP_IPC_MASTER_HPP_
