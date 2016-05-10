
#ifndef _EQP_PROTOCOL_HANDLER_HPP_
#define _EQP_PROTOCOL_HANDLER_HPP_

#include "define.hpp"
#include "netcode.hpp"
#include "eq_packet_protocol.hpp"
#include "packet_structs_protocol.hpp"
#include "aligned.hpp"
#include "clock.hpp"
#include <sqlite3.h>

class UdpSocket;

class ProtocolHandler
{
private:
    IpAddress   m_address;
    UdpSocket&  m_socket;
    uint32_t    m_sessionId;    // This is stored in network byte order
    uint32_t    m_crcKey;
    uint64_t    m_packetsSent;
    uint64_t    m_packetsReceived;
    uint64_t    m_startTimeMilliseconds;    // For SessionStatResponse
    
private:
    void handleSessionRequest(AlignedReader& r);
    void handleSessionDisconnect();
    void handleSessionStatsRequest(AlignedReader& r);
    void handleCombined(AlignedReader& r);

    void receive(byte* data, uint32_t len, bool isFromCombined);
    
public:
    ProtocolHandler(IpAddress& addr, UdpSocket& socket);
    virtual ~ProtocolHandler();
    
    bool receive(byte* data, uint32_t len);
    void sendImmediate(const void* data, uint32_t len);
    void disconnect();
};

#endif//_EQP_PROTOCOL_HANDLER_HPP_
