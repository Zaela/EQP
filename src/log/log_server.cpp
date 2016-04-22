
#include "log_server.hpp"

#include "clock.hpp"
static int written = 0;
static int _read = 0;
static uint64_t totalTime = 0;

LogServer::LogServer()
: m_mainEnd(false),
  m_threadEnd(false),
  m_threadQueueStart(0),
  m_threadQueueEnd(0)
{
    m_shareMemViewer.open(EQP_LOG_SHARE_MEM_NAME, LogShareMem::SIZE);
    m_shareMemQueue = (LogShareMem*)m_shareMemViewer.getMemory();
    
    if (!m_shareMemQueue)
        throw 1; //fixme
    
    
    m_testFile = fopen("test.log", "w+");
}

LogServer::~LogServer()
{
    if (m_testFile)
        ::fclose(m_testFile);
    
    printf("written: %i, read: %i\n", written, _read);
    printf("total time: %lu\n", totalTime);
}

void LogServer::startThread()
{
    std::thread thread(threadProc, this);
    thread.detach();
}

void LogServer::mainLoop()
{
    for (;;)
    {
        // Block until the producer informs us that there are messages available
        m_shareMemQueue->wait();
        
        // If we're here, we have messages; read them until we run out
        for (;;)
        {
            SharedRingBuffer::Packet packet;
            
            if (!m_shareMemQueue->pop(packet))
                break;
            
            ++_read;
            
            // Attempt to add to the thread ring queue
            if (pushMessage(packet))
                continue;

            // If we're here, there was no space in the ring queue
            // Add to the pending queue instead
            m_pendingMessages.emplace_back(std::move(packet));
        }
        
        // Wake the thread
        m_threadSemaphore.trigger();
        
        // If we have pending messages, wait for the thread to do a little work, then try again...
        if (!m_pendingMessages.empty())
        {
            for (SharedRingBuffer::Packet& p : m_pendingMessages)
            {
                for (;;)
                {
                    if (!pushMessage(p))
                    {
                        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }
                    
                    m_threadSemaphore.trigger();
                    break;
                }
            }
            
            m_pendingMessages.clear();
        }
        
        if (m_mainEnd)
        {
            waitForThreadToFinish();
            return;
        }
    }
}

bool LogServer::pushMessage(SharedRingBuffer::Packet& packet)
{
    if (packet.opcode() == ServerOp::Shutdown)
        m_mainEnd = true;
    
    uint32_t startOffset    = m_threadQueueStart.load();
    uint32_t endOffset      = m_threadQueueEnd.load();
    
    uint32_t check = endOffset + 1;
    if (check == MAX_QUEUE)
        check = 0;
    
    if (check == startOffset)
        return false;
    
    m_threadQueue[endOffset] = std::move(packet);
    
    m_threadQueueEnd.store(check);
    
    return true;
}

SharedRingBuffer::Packet* LogServer::popMessage()
{
    uint32_t startOffset    = m_threadQueueStart.load();
    uint32_t endOffset      = m_threadQueueEnd.load();
    
    if (startOffset == endOffset)
        return nullptr;
    
    return &m_threadQueue[startOffset];
}

void LogServer::threadLoop()
{
    for (;;)
    {
        // Block until the producer (mainLoop) informs us that there are messages available
        m_threadSemaphore.wait();
        
        uint32_t x = 0;
        uint64_t t = Clock::microseconds();
        
        // If we're here, we have messages; read them until we run out
        for (;;)
        {
            SharedRingBuffer::Packet* p = popMessage();
            
            if (!p)
                break;
            
            if (p->opcode() != ServerOp::Shutdown)
            { 
                //testtesttest
                if (p->data())
                {
                    fwrite(p->data(), sizeof(byte), p->length(), m_testFile);
                    fputc(' ', m_testFile);
                    fputc('0' + p->sourceId(), m_testFile);
                    fputc('\n', m_testFile);
                }
                x++;
                ++written;
                //testtesttest
            }
            else
            {
                m_threadEnd = true;
            }
            
            p->~Packet();
            
            uint32_t startOffset = m_threadQueueStart.load() + 1;
            
            if (startOffset == MAX_QUEUE)
                startOffset = 0;
            
            m_threadQueueStart.store(startOffset);
        }
        
        if (x)
            totalTime += Clock::microseconds() - t;
        
        if (m_threadEnd)
            return;
    }
}

void LogServer::threadProc(LogServer* ls)
{
    std::lock_guard<AtomicMutex> lock(ls->m_threadLifetimeMutex);
    ls->threadLoop();
}

void LogServer::waitForThreadToFinish()
{
    std::lock_guard<AtomicMutex> lock(m_threadLifetimeMutex);
}
