
#include "protocol_handler.hpp"
#include "udp_socket.hpp"

ProtocolHandler::ProtocolHandler(IpAddress& addr, UdpSocket& socket)
: m_address(addr),
  m_socket(socket),
  m_sessionId(0),
  m_crcKey(0),
  m_startTimeMilliseconds(Clock::milliseconds())
{
    
}

ProtocolHandler::~ProtocolHandler()
{
    
}

bool ProtocolHandler::receive(byte* data, uint32_t len)
{
    m_packetsReceived++;
    receive(data, len, false);
    return false; //return hasInputPacketsReadyToRead();
}

void ProtocolHandler::receive(byte* data, uint32_t len, bool isFromCombined)
{
    AlignedReader r(data, len);
    
    for (uint32_t i = 0; i < len; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");
    
    uint16_t opcode = toHostShort(r.uint16());
    
    if (opcode > 0xff)
    {
        // Raw, unordered packet, no protocol
        //handle
        return;
    }
    
    switch (opcode)
    {
    case EQProtocol::SessionRequest:
        handleSessionRequest(r);
        break;
    
    case EQProtocol::SessionDisconnect:
        handleSessionDisconnect();
        break;
    
    case EQProtocol::SessionStatsRequest:
        handleSessionStatsRequest(r);
        break;
    
    case EQProtocol::Combined:
        handleCombined(r);
        break;
    
    default:
        break;
    }
}

void ProtocolHandler::sendImmediate(const void* data, uint32_t len)
{
    m_packetsSent++;
    m_socket.sendImmediate(data, len, m_address);
}

void ProtocolHandler::handleSessionRequest(AlignedReader& r)
{
    // Opcode size is counted in the struct, unlike most packets
    if (r.size() < sizeof(ProtocolStruct::SessionRequest))
        return;
    
    // unknown
    r.advance(sizeof(uint32_t));
    // session
    m_sessionId = r.uint32();
    
    ProtocolStruct::SessionResponse resp;
    AlignedWriter w(&resp, sizeof(ProtocolStruct::SessionResponse));
    
    // opcode
    w.uint16(toNetworkShort(EQProtocol::SessionResponse));
    // session
    w.uint32(m_sessionId);
    // crcKey
    sqlite3_randomness(sizeof(uint32_t), &m_crcKey);
    w.uint32(toNetworkLong(m_crcKey));
    // validation
    w.uint8(0);
    // format
    w.uint8(0);//ProtocolStruct::SessionRequest::Format::Compressed);
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
    m_packetsSent++;
    
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
    resp.packetsSent            = toNetworkUint64(m_packetsSent);
    resp.packetsReceived        = toNetworkUint64(m_packetsReceived);
    
    m_socket.sendImmediate(&resp, sizeof(ProtocolStruct::SessionStatsServer), m_address);
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

void ProtocolHandler::disconnect()
{
    ProtocolStruct::SessionDisconnect dis;
    AlignedWriter w(&dis, sizeof(ProtocolStruct::SessionDisconnect));
    
    // opcode
    w.uint16(toNetworkShort(EQProtocol::SessionDisconnect));
    // session
    w.uint32(m_sessionId);
    
    m_socket.sendImmediate(&dis, sizeof(ProtocolStruct::SessionDisconnect), m_address);
    m_socket.removeHandler(this);
}
