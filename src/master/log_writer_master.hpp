
#ifndef _EQP_LOG_WRITER_MASTER_HPP_
#define _EQP_LOG_WRITER_MASTER_HPP_

#include "define.hpp"
#include "log_writer.hpp"
#include "atomic_mutex.hpp"
#include "semaphore.hpp"
#include "ring_buffer.hpp"
#include "server_op.hpp"
#include "source_id.hpp"
#include "exception.hpp"
#include <vector>
#include <string>
#include <ctime>
#include <time.h>

class LogWriterMaster : public LogWriter
{
private:
    enum Opcode
    {
        Write,
        Open,
        Close
    };
    
    struct Op
    {
        Opcode                      opcode;
        SharedRingBuffer::Packet    packet;
        
        Op(Opcode op) : opcode(op) { }
        Op(Opcode op, SharedRingBuffer::Packet& p) : opcode(op), packet(std::move(p)) { }
        Op(Op&& o) { *this = std::move(o); }
        
        Op& operator=(Op&& o)
        {
            opcode = o.opcode;
            packet = std::move(o.packet);
            return *this;
        }
    };
    
    struct LogFile
    {
        int     sourceId;
        FILE*   file;
    };
    
private:
    std::vector<LogFile> m_logFiles;

    bool                m_threadEnd;
    Semaphore           m_threadSemaphore;
    AtomicMutex         m_inQueueMutex;
    std::vector<Op>     m_inQueue;

    AtomicMutex         m_threadLifetimeMutex;
    std::vector<Op>     m_threadProcessQueue;

private:
    void threadLoop();
    static void threadProc(LogWriterMaster* writer);
    void write(int sourceId, SharedRingBuffer::Packet& packet);
    void determineLogFileNameAndOpen(int sourceId);
    void close(int sourceId);
    void close(FILE* fp);
    void closeAll();

    void schedule(Op& op);
    
public:
    LogWriterMaster();
    ~LogWriterMaster();

    void init();

    void openLogFileFor(int sourceId);
    void closeLogFileFor(int sourceId);

    virtual void log(Log::Type type, const char* fmt, ...);
    virtual void log(Log::Type type, const char* fmt, va_list args);
    void log(SharedRingBuffer::Packet& packet);
};

#endif//_EQP_LOG_WRITER_HPP_