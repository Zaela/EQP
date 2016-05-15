
#ifndef _EQP_PROTOCOL_HANDLER_HPP_
#define _EQP_PROTOCOL_HANDLER_HPP_

#include "define.hpp"
#include "netcode.hpp"
#include "eq_packet_protocol.hpp"
#include "packet_structs_protocol.hpp"
#include "aligned.hpp"
#include "clock.hpp"
#include "crc.hpp"
#include "ack_manager.hpp"
#include "random.hpp"
#include <zlib.h>

class ProtocolHandler : public AckManager
{
private:
    uint32_t    m_crcKey;
    uint64_t    m_startTimeMilliseconds;    // For SessionStatResponse
    
private:
    void handleSessionRequest(AlignedReader& r);
    void handleSessionDisconnect();
    void handleSessionStatsRequest(AlignedReader& r);
    void handleCombined(AlignedReader& r);

    bool validateAndDecompressPacket(AlignedReader& r, bool isFromCombined);
    bool decompressPacket(AlignedReader& r);

    void receive(byte* data, uint32_t len, bool isFromCombined);
    
public:
    ProtocolHandler(IpAddress& addr, UdpSocket& socket);
    virtual ~ProtocolHandler();
    
    bool receive(byte* data, uint32_t len);
    void disconnect();
};

#endif//_EQP_PROTOCOL_HANDLER_HPP_
