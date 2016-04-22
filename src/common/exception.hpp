
#ifndef _EQP_EXCEPTION_HPP_
#define _EQP_EXCEPTION_HPP_

#include "define.hpp"
#include <exception>

class Exception : public std::exception
{
public:
    static const uint32_t MAX_LENGTH = 2048;

private:
    char m_message[MAX_LENGTH];

public:
    Exception(const char* fmt, ...);
    virtual ~Exception() { }
    
    virtual const char* what() const noexcept { return m_message; }
};

#endif//_EQP_EXCEPTION_HPP_
