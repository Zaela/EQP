
#include "log_writer_common.hpp"

void LogWriterCommon::log(Log::Type type, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogWriterCommon::log(type, fmt, args);
    va_end(args);
}

void LogWriterCommon::log(Log::Type type, const char* fmt, va_list args)
{
    char message[LogWriter::MESSAGE_SIZE];
    
    uint32_t len = constructMessage(message, type, fmt, args);
    
    if (len)
        m_ipc.push(ServerOp::LogMessage, m_sourceId, len, (byte*)message);
}
