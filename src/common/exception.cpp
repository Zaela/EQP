
#include "exception.hpp"

Exception::Exception(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    ::vsnprintf(m_message, MAX_LENGTH, fmt, args);
    va_end(args);
}
