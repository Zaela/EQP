
#ifndef _EQP_PACKET_TRACKER_HPP_
#define _EQP_PACKET_TRACKER_HPP_

#include "define.hpp"
#include "netcode.hpp"

class UdpSocket;

class PacketTracker
{
private:
    int         m_socketFd;
    IpAddress   m_address;
    UdpSocket&  m_socket;
    uint64_t    m_packetsSent;
    uint64_t    m_packetsReceived;
    
protected:
    void incrementPacketsSent() { m_packetsSent++; }
    void incrementPacketsReceived() { m_packetsReceived++; }
    
    UdpSocket& socket() { return m_socket; }
    uint64_t packetsSent() const { return m_packetsSent; }
    uint64_t packetsReceived() const { return m_packetsReceived; }
    
    void sendImmediateNoIncrement(const void* data, uint32_t len);
    
public:
    PacketTracker(IpAddress& addr, UdpSocket& socket);
    virtual ~PacketTracker();

    void sendImmediate(const void* data, uint32_t len);
};

#endif//_EQP_PACKET_TRACKER_HPP_
