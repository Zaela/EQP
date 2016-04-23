
#include "log_writer.hpp"

uint32_t LogWriter::constructMessage(char* message, Log::Type type, const char* fmt, va_list args)
{
    time_t rawTime      = time(nullptr);
    struct tm* curTime  = localtime(&rawTime);
    
    size_t pos = strftime(message, MESSAGE_SIZE, "[%Y-%m-%d : %H:%M:%S]", curTime);
    
    if (pos == 0)
        throw Exception("[LogWriter::constructMessage] strftime() failed");
    
    int wrote = 0;
    
    switch (type)
    {
    case Log::Fatal:
        wrote = snprintf(message + pos, MESSAGE_SIZE - pos, "[FATAL] ");
        break;
    
    case Log::Error:
        wrote = snprintf(message + pos, MESSAGE_SIZE - pos, "[ERROR] ");
        break;
    
    case Log::Info:
        wrote = snprintf(message + pos, MESSAGE_SIZE - pos, "[INFO] ");
        break;
    
    case Log::SQL:
        wrote = snprintf(message + pos, MESSAGE_SIZE - pos, "[SQL] ");
        break;
    
    case Log::None:
        wrote = snprintf(message + pos, MESSAGE_SIZE - pos, " ");
        break;
    }
    
    if (wrote > 0 && (size_t)wrote < (MESSAGE_SIZE - pos))
    {
        pos += (size_t)wrote;
        
        wrote = vsnprintf(message + pos, MESSAGE_SIZE - pos, fmt, args);
        
        if (wrote > 0 && (size_t)wrote < (MESSAGE_SIZE - pos))
        {
            pos += (size_t)wrote;
            return pos;
        }
    }
    
    return 0;
}
