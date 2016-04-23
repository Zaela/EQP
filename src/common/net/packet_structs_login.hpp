
#ifndef _EQP_PACKET_STRUCTS_LOGIN_HPP_
#define _EQP_PACKET_STRUCTS_LOGIN_HPP_

#include "define.hpp"

namespace LoginStruct
{
#pragma pack(1)
    // Login packets need interoperability with the EQEmu login server
    struct Request
    {
        uint32_t accountId;
        uint32_t serverId;
        // These don't appear to be used
        uint32_t fromId;
        uint32_t toId;
    };
        
    struct Response
    {
        uint32_t    accountId;
        uint32_t    serverId;
        int8_t      response; // 1 = Allowed, 0 = Denied, -1 = Suspended, -2 = Banned, -3 = World Full
        // These don't appear to be used
        uint32_t    fromId;
        uint32_t    toId;
    };
#pragma pack()
    
}; //namespace Packet

#endif//_EQP_PACKET_STRUCTS_LOGIN_HPP_
