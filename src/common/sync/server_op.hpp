
#ifndef _EQP_SERVER_OP_HPP_
#define _EQP_SERVER_OP_HPP_

#include "define.hpp"

enum class ServerOp : uint32_t
{
    None,
    // Common
    Shutdown,
    LogMessage,
    // ZoneCluster
    // Zone
    // CharSelect
    // Login
    LoginRequest,
    LoginResponse,
    COUNT
};

#endif//_EQP_SERVER_OP_HPP_
