
#ifndef _EQP_PACKET_STRUCTS_PROTOCOL_HPP_
#define _EQP_PACKET_STRUCTS_PROTOCOL_HPP_

#include "define.hpp"

namespace ProtocolStruct
{
#pragma pack(1)
    struct SessionRequest
    {
        uint16_t    opcode;
        uint32_t    unknown;
        uint32_t    session;
        uint32_t    maxLength;
    };
    
    struct SessionResponse
    {
        uint16_t    opcode;
        uint32_t    session;
        uint32_t    crcKey;
        uint8_t     validation;
        uint8_t     format;
        uint8_t     unknownA;
        uint32_t    maxLength;
        uint32_t    unknownB;
        
        enum Format
        {
            Compressed  = 0x01,
            Encoded     = 0x04
        };
    };
    
    struct SessionStatsClient
    {
        uint16_t    opcode;
        uint16_t    requestId;
        uint32_t    lastLocalDelta;
        uint32_t    averageDelta;
        uint32_t    lowDelta;
        uint32_t    highDelta;
        uint32_t    lastRemoteDelta;
        uint64_t    packetsSent;
        uint64_t    packetsReceived;
    };
    
    struct SessionStatsServer
    {
        uint16_t    opcode;
        uint16_t    requestId;
        uint32_t    serverTime;
        uint64_t    packetsSentEcho;
        uint64_t    packetsReceivedEcho;
        uint64_t    packetsSent;
        uint64_t    packetsReceived;
    };

    struct SessionDisconnect
    {
        uint16_t    opcode;
        uint32_t    sessionId;
    };
#pragma pack()
    
}; //namespace ProtocolStruct

#endif//_EQP_PACKET_STRUCTS_PROTOCOL_HPP_
