
#ifndef _EQP_LOG_WRITER_COMMON_HPP_
#define _EQP_LOG_WRITER_COMMON_HPP_

#include "define.hpp"
#include "log_writer.hpp"
#include "ipc_buffer.hpp"
#include "server_op.hpp"

class LogWriterCommon : public LogWriter
{
private:
    int         m_sourceId;
    IpcRemote&  m_ipc;
    
public:
    LogWriterCommon(int sourceId, IpcRemote& ipc) : m_sourceId(sourceId), m_ipc(ipc) { }

    virtual void log(Log::Type type, const char* fmt, ...);
    virtual void log(Log::Type type, const char* fmt, va_list args);
};

#endif//_EQP_LOG_WRITER_COMMON_HPP_
