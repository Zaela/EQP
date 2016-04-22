
#include "log_writer_master.hpp"

LogWriterMaster::LogWriterMaster()
: m_threadEnd(false)
{
    
}

LogWriterMaster::~LogWriterMaster()
{
    closeLogFileFor(SourceId::Master);
    
    m_threadEnd = true;
    m_threadSemaphore.trigger();
    std::lock_guard<AtomicMutex> lock(m_threadLifetimeMutex);
}

void LogWriterMaster::init()
{
    std::thread thread(threadProc, this);
    thread.detach();
    
    openLogFileFor(SourceId::Master);
    log(Log::Info, "LogWriterMaster initialized");
}

void LogWriterMaster::openLogFileFor(int sourceId)
{
    SharedRingBuffer::Packet packet(ServerOp::None, sourceId, 0, nullptr);
    Op op(Opcode::Open, packet);
    schedule(op);
    
    log(Log::None, "==== Log File Opened ====", sourceId);
}

void LogWriterMaster::closeLogFileFor(int sourceId)
{
    SharedRingBuffer::Packet packet(ServerOp::None, sourceId, 0, nullptr);
    Op op(Opcode::Close, packet);
    schedule(op);
}

void LogWriterMaster::log(Log::Type type, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogWriterMaster::log(type, fmt, args);
    va_end(args);
}

void LogWriterMaster::log(Log::Type type, const char* fmt, va_list args)
{
    char message[2048];
    
    time_t rawTime      = time(nullptr);
    struct tm* curTime  = localtime(&rawTime);
    
    size_t pos = strftime(message, sizeof(message), "[%c]", curTime);
    
    if (pos == 0)
        throw Exception("[LogWriterMaster::log] strftime() failed");
    
    int wrote = 0;
    
    switch (type)
    {
    case Log::Fatal:
        wrote = snprintf(message + pos, sizeof(message) - pos, "[FATAL] ");
        break;
    case Log::Error:
        wrote = snprintf(message + pos, sizeof(message) - pos, "[ERROR] ");
        break;
    case Log::Info:
        wrote = snprintf(message + pos, sizeof(message) - pos, "[INFO] ");
        break;
    case Log::SQL:
        wrote = snprintf(message + pos, sizeof(message) - pos, "[SQL] ");
        break;
    case Log::None:
        wrote = snprintf(message + pos, sizeof(message) - pos, " ");
        break;
    }
    
    if (wrote > 0 && (size_t)wrote < (sizeof(message) - pos))
    {
        pos += (size_t)wrote;
        
        wrote = vsnprintf(message + pos, sizeof(message) - pos, fmt, args);
        
        if (wrote > 0 && (size_t)wrote < (sizeof(message) - pos))
        {
            pos += (size_t)wrote;
            
            SharedRingBuffer::Packet packet(ServerOp::None, SourceId::Master, pos, (byte*)message);
            log(packet);
        }
    }
}

void LogWriterMaster::log(SharedRingBuffer::Packet& packet)
{
    Op op(Opcode::Write, packet);
    schedule(op);
}

void LogWriterMaster::schedule(Op& op)
{
    m_inQueueMutex.lock();
    m_inQueue.emplace_back(std::move(op));
    m_inQueueMutex.unlock();
    m_threadSemaphore.trigger();
}

/*================================================================================*\
** Background logging thread methods
\*================================================================================*/

void LogWriterMaster::threadProc(LogWriterMaster* writer)
{
    std::lock_guard<AtomicMutex> lock(writer->m_threadLifetimeMutex);
    writer->threadLoop();
}

void LogWriterMaster::threadLoop()
{
    log(Log::Info, "Logging Thread started");
    
    for (;;)
    {
        // Block until an operation is scheduled or thread is flagged to end
        m_threadSemaphore.wait();
        
        m_inQueueMutex.lock();
        if (!m_inQueue.empty())
        {
            for (Op& op : m_inQueue)
            {
                m_threadProcessQueue.emplace_back(std::move(op));
            }
            
            m_inQueue.clear();
        }
        m_inQueueMutex.unlock();
        
        if (!m_threadProcessQueue.empty())
        {
            for (Op& op : m_threadProcessQueue)
            {
                int sourceId = op.packet.sourceId();
                
                switch (op.opcode)
                {
                case Write:
                    write(sourceId, op.packet);
                    break;
                case Open:
                    determineLogFileNameAndOpen(sourceId);
                    break;
                case Close:
                    close(sourceId);
                    break;
                }
            }
            
            m_threadProcessQueue.clear();
        }
        
        if (m_threadEnd)
        {
            closeAll();
            return;
        }
    }
}

void LogWriterMaster::write(int sourceId, SharedRingBuffer::Packet& packet)
{
    FILE* fp = nullptr;
    
    for (LogFile& lf : m_logFiles)
    {
        if (lf.sourceId == sourceId)
        {
            fp = lf.file;
            break;
        }
    }
    
    if (fp)
    {
        ::fwrite(packet.data(), sizeof(byte), packet.length(), fp);
        ::fputc('\n', fp);
    }
}

void LogWriterMaster::determineLogFileNameAndOpen(int sourceId)
{
    char filename[256];
    
    switch (sourceId)
    {
    case SourceId::Master:
        ::snprintf(filename, 256, "log/master.log");
        break;
    case SourceId::CharSelect:
        ::snprintf(filename, 256, "log/char_select.log");
        break;
    default:
        if (sourceId >= SourceId::ZoneClusterOffset)
            ::snprintf(filename, 256, "log/zone_cluster%i.log", sourceId - SourceId::ZoneClusterOffset);
        else
            ::snprintf(filename, 256, "log/zone%i.log", sourceId);
        break;
    }
    
    //fixme: add logic here to check for existing large logs, to be compressed, stashed away and replaced fresh
    FILE* fp = ::fopen(filename, "a");
    
    if (!fp)
    {
        // Log the failure in master.log, assuming we haven't failed to open it...
        char message[2048];
    
        time_t rawTime      = time(nullptr);
        struct tm* curTime  = localtime(&rawTime);
        
        size_t pos = strftime(message, sizeof(message), "[%c][ERROR][LogWriterMaster::determineLogFileNameAndOpen] Could not open file for appending: ", curTime);
        
        if (pos != 0)
        {
            int wrote = snprintf(message, sizeof(message) - pos, "'%s'", filename);
            
            if (wrote > 0 && (size_t)wrote < (sizeof(message) - pos))
            {
                pos += (size_t)wrote;
                
                SharedRingBuffer::Packet packet(ServerOp::None, SourceId::Master, pos, (byte*)message);
                m_threadProcessQueue.emplace_back(Opcode::Write, packet);
            }
        }
    }
    
    LogFile lf;
    
    lf.sourceId = sourceId;
    lf.file     = fp;
    
    m_logFiles.push_back(lf);
}

void LogWriterMaster::close(int sourceId)
{
    for (uint32_t i = 0; i < m_logFiles.size(); i++)
    {
        LogFile& lf = m_logFiles[i];
        
        if (lf.sourceId == sourceId)
        {
            close(lf.file);
            
            // Swap and pop
            if (i != (m_logFiles.size() - 1))
            {
                m_logFiles[i] = std::move(m_logFiles.back());
            }
            
            m_logFiles.pop_back();
            return;
        }
    }
}

void LogWriterMaster::close(FILE* fp)
{
    char message[2048];
    
    time_t rawTime      = time(nullptr);
    struct tm* curTime  = localtime(&rawTime);
    
    size_t pos = strftime(message, sizeof(message), "[%c] ==== Log File Closed ====\n", curTime);
    
    if (pos != 0)
        ::fwrite(message, sizeof(byte), pos, fp);
    
    ::fclose(fp);
}

void LogWriterMaster::closeAll()
{
    for (LogFile& lf : m_logFiles)
    {
        close(lf.file);
    }
}
