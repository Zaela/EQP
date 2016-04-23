
#ifndef _EQP_LOG_WRITER_HPP_
#define _EQP_LOG_WRITER_HPP_

#include "define.hpp"
#include "terminal.hpp"
#include "exception.hpp"
#include <ctime>
#include <time.h>

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
    static const uint32_t MESSAGE_SIZE = 2048;

protected:
    uint32_t constructMessage(char* message, Log::Type type, const char* fmt, va_list args);

public:
    virtual void log(Log::Type type, const char* fmt, ...) = 0;
    virtual void log(Log::Type type, const char* fmt, va_list args) = 0;
};

#endif//_EQP_LOG_WRITER_HPP_
