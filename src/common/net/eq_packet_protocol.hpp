
#ifndef _EQP_EQ_PACKET_PROTOCOL_HPP_
#define _EQP_EQ_PACKET_PROTOCOL_HPP_

#include "define.hpp"

enum EQProtocol : uint16_t
{
    None                = 0x0000,
    SessionRequest      = 0x0001,
    SessionResponse     = 0x0002,
    Combined            = 0x0003,
    SessionDisconnect   = 0x0005,
    KeepAlive           = 0x0006,
    SessionStatRequest  = 0x0007,
    SessionStatResponse = 0x0008,
    Packet              = 0x0009,
    Fragment            = 0x000d,
    OutOfOrder          = 0x0011,
    Ack                 = 0x0015,
};

#endif//_EQP_EQ_PACKET_PROTOCOL_HPP_
