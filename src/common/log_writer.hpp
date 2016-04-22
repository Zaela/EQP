
#ifndef _EQP_LOG_WRITER_HPP_
#define _EQP_LOG_WRITER_HPP_

#include "define.hpp"

class Log
{
public:
    enum Type
    {
        None,
        Fatal,
        Error,
        Info,
        SQL
    };
};

class LogWriter
{
public:
    virtual void log(Log::Type type, const char* fmt, ...) = 0;
    virtual void log(Log::Type type, const char* fmt, va_list args) = 0;
};

#endif//_EQP_LOG_WRITER_HPP_
