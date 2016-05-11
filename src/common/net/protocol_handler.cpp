
#include "protocol_handler.hpp"
#include "udp_socket.hpp"

ProtocolHandler::ProtocolHandler(IpAddress& addr, UdpSocket& socket)
: AckManager(addr, socket),
  m_crcKey(0),
  m_startTimeMilliseconds(Clock::milliseconds())
{
    
}

ProtocolHandler::~ProtocolHandler()
{
    
}

bool ProtocolHandler::receive(byte* data, uint32_t len)
{
    incrementPacketsReceived();
    receive(data, len, false);
    return hasInputPacketsQueued();
}

void ProtocolHandler::receive(byte* data, uint32_t len, bool isFromCombined)
{
    AlignedReader r(data, len);
    
    printf("<- ");
    for (uint32_t i = 0; i < len; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");
    
    uint16_t opcode = toHostShort(r.uint16());
    
    if (opcode > 0xff)
    {
        // Raw, unordered packet, no protocol
        queueInputPacket(data, len);
        return;
    }
    
    // Packets that aren't subject to verification checks
    switch (opcode)
    {
    case EQProtocol::SessionRequest:
        handleSessionRequest(r);
        return;
    
    case EQProtocol::SessionDisconnect:
        handleSessionDisconnect();
        return;
    
    default:
        break;
    }
    
    if (!validateAndDecompressPacket(r, isFromCombined))
        return;
    
    switch (opcode)
    {
    case EQProtocol::SessionStatsRequest:
        handleSessionStatsRequest(r);
        break;
    
    case EQProtocol::Combined:
        handleCombined(r);
        break;
        
    case EQProtocol::Packet:
        checkSequencePacket(r);
        break;
    
    case EQProtocol::Fragment:
        checkSequenceFragment(r);
        break;
    
    default:
        break;
    }
}

void ProtocolHandler::handleSessionRequest(AlignedReader& r)
{
    // Opcode size is counted in the struct, unlike most packets
    if (r.size() < sizeof(ProtocolStruct::SessionRequest))
        return;
    
    // unknown
    r.advance(sizeof(uint32_t));
    // session
    setSessionId(r.uint32());
    
    ProtocolStruct::SessionResponse resp;
    AlignedWriter w(&resp, sizeof(ProtocolStruct::SessionResponse));
    
    // opcode
    w.uint16(toNetworkShort(EQProtocol::SessionResponse));
    // session
    w.uint32(sessionId());
    // crcKey
    sqlite3_randomness(sizeof(uint32_t), &m_crcKey);
    w.uint32(toNetworkLong(m_crcKey));
    // validation
#ifdef EQP_DISABLE_PACKET_CRC
    w.uint8(0);
#else
    w.uint8(ProtocolStruct::SessionResponse::Validation::CRC);
#endif
    // format
#ifdef EQP_DISABLE_PACKET_COMPRESSION
    w.uint8(0);
#else
    w.uint8(ProtocolStruct::SessionResponse::Format::Compressed);
#endif
    // unknownA
    w.uint8(0);
    // maxLength
    w.uint32(r.uint32());
    
    sendImmediate(&resp, sizeof(ProtocolStruct::SessionResponse));
}

void ProtocolHandler::handleSessionDisconnect()
{
    disconnect();
}

void ProtocolHandler::handleSessionStatsRequest(AlignedReader& r)
{
    if (r.size() < sizeof(ProtocolStruct::SessionStatsClient))
        return;
    
    // The response counts itself as a packet sent
    incrementPacketsSent();
    
    // Both structs just happens to be perfectly aligned along 8-byte boundaries
    // (But we copy the request just in case it was part of a combined packet...)
    ProtocolStruct::SessionStatsClient req;
    memcpy(&req, r.all(), sizeof(ProtocolStruct::SessionStatsServer));
    
    ProtocolStruct::SessionStatsServer resp;
    
    resp.opcode                 = toNetworkShort(EQProtocol::SessionStatsResponse);
    resp.requestId              = 0;    // Would expect to echo the requestId from the client, but eqemu has it commented out...
    resp.serverTime             = (uint32_t)(Clock::milliseconds() - m_startTimeMilliseconds);
    resp.packetsSentEcho        = req.packetsSent;
    resp.packetsReceivedEcho    = req.packetsReceived;
    resp.packetsSent            = toNetworkUint64(packetsSent());
    resp.packetsReceived        = toNetworkUint64(packetsReceived());
    
    //fixme: need to compress and crc this packet
    sendImmediateNoIncrement(&resp, sizeof(ProtocolStruct::SessionStatsServer));
}

void ProtocolHandler::handleCombined(AlignedReader& r)
{
    while (r.remaining())
    {
        uint32_t size = r.uint8();
        
        if (size > r.remaining())
            break;
        
        receive(r.current(), size, true);
        r.advance(size);
    }
}

bool ProtocolHandler::validateAndDecompressPacket(AlignedReader& r, bool isFromCombined)
{
#ifndef EQP_DISABLE_PACKET_CRC
    if (!isFromCombined)
    {
        if (!CRC16::validatePacket(r.all(), r.size(), m_crcKey))
            return false;
        // Don't count the CRC in the length hereafter
        r.reduceSize(sizeof(uint16_t));
    }
#endif
    
    byte flag = r.peekByte();
    
    if (flag == 0x5a) // Compressed
    {
        if (!decompressPacket(r))
            return false;
    }
    else if (flag == 0xa5) // Explicitly not compressed
    {
        r.advance(sizeof(byte)); // Skip the flag byte
    }
    
    return true;
}

bool ProtocolHandler::decompressPacket(AlignedReader& r)
{
    r.reset();
    
    byte* buffer = socket().getDecompressionBuffer();
    // Write the protocol opcode into the first two bytes of the buffer
    // Keeps things consistent, allows us to avoid some oddball special cases
    *(uint16_t*)buffer = r.uint16();
    
    r.advance(sizeof(byte)); // Compressed flag
    
    unsigned long bLength = UdpSocket::BUFFER_SIZE - 2;
    
    if (uncompress(buffer + 2, &bLength, r.current(), r.remaining()) != Z_OK)
        return false;
    
    r.reset(buffer, (uint32_t)bLength + 2);
    r.advance(sizeof(uint16_t)); // Protocol opcode
    
    return true;
}

void ProtocolHandler::disconnect()
{
    ProtocolStruct::SessionDisconnect dis;
    AlignedWriter w(&dis, sizeof(ProtocolStruct::SessionDisconnect));
    
    // opcode
    w.uint16(toNetworkShort(EQProtocol::SessionDisconnect));
    // session
    w.uint32(sessionId());
    
    sendImmediate(&dis, sizeof(ProtocolStruct::SessionDisconnect));
    socket().removeHandler(this);
    clearPacketQueues();
}
