
#ifndef _EQP_SERVER_OP_HPP_
#define _EQP_SERVER_OP_HPP_

#include "define.hpp"

enum class ServerOp : uint32_t
{
    None,
    Shutdown,
    LogMessage
};

#endif//_EQP_SERVER_OP_HPP_
