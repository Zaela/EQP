
#include "log_writer_common.hpp"

void LogWriterCommon::log(Log::Type type, const char* fmt, ...)
{
    (void)type;
    (void)fmt;
}

void LogWriterCommon::log(Log::Type type, const char* fmt, va_list args)
{
    (void)type;
    (void)fmt;
    (void)args;
}
