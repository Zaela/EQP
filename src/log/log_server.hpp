
#ifndef _EQP_LOG_SERVER_HPP_
#define _EQP_LOG_SERVER_HPP_

#include "define.hpp"
#include "share_mem.hpp"
#include "log_share_mem.hpp"
#include "semaphore.hpp"
#include "atomic_mutex.hpp"
#include "server_op.hpp"
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>

class LogServer
{
private:
    static const uint32_t MAX_QUEUE = 256;

private:
    LogShareMem*    m_shareMemQueue;
    ShareMemViewer  m_shareMemViewer;

    bool m_mainEnd;

    Semaphore   m_threadSemaphore;
    AtomicMutex m_threadLifetimeMutex;

    std::vector<SharedRingBuffer::Packet> m_pendingMessages;

    bool                        m_threadEnd;
    std::atomic<uint32_t>       m_threadQueueStart;
    std::atomic<uint32_t>       m_threadQueueEnd;
    SharedRingBuffer::Packet    m_threadQueue[MAX_QUEUE];

    FILE* m_testFile;

    //std::unordered_map<int, FILE*> m_filesBySourceId;

private:
    bool pushMessage(SharedRingBuffer::Packet& packet);
    SharedRingBuffer::Packet* popMessage();
    void threadLoop();
    void waitForThreadToFinish();
    static void threadProc(LogServer* server);

public:
    LogServer();
    ~LogServer();

    void startThread();

    void mainLoop();
};

#endif//_EQP_LOG_SERVER_HPP_
