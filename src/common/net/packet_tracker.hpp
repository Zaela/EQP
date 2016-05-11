
#ifndef _EQP_PACKET_TRACKER_HPP_
#define _EQP_PACKET_TRACKER_HPP_

#include "define.hpp"
#include "netcode.hpp"
#include <vector>

class UdpSocket;

class PacketTracker
{
protected:
    struct InputPacket
    {
        byte*       data;
        uint32_t    length;
        
        InputPacket(byte* d, uint32_t l) : data(d), length(l) { }
        InputPacket(InputPacket&& o) : data(o.data), length(o.length) { o.data = nullptr; }
        
        ~InputPacket()
        {
            if (data)
            {
                delete[] data;
                data = nullptr;
            }
        }
    };

private:
    int         m_socketFd;
    IpAddress   m_address;
    UdpSocket&  m_socket;
    uint64_t    m_packetsSent;
    uint64_t    m_packetsReceived;
    uint32_t    m_sessionId;    // This is stored in network byte order
    uint32_t    m_accountId;

    std::vector<InputPacket>    m_inputPacketQueue;
    
protected:
    void incrementPacketsSent() { m_packetsSent++; }
    void incrementPacketsReceived() { m_packetsReceived++; }
    
    uint32_t ipAddress() const { return m_address.sin_addr.s_addr; }
    uint16_t port() const { return m_address.sin_port; }
    UdpSocket& socket() { return m_socket; }
    uint64_t packetsSent() const { return m_packetsSent; }
    uint64_t packetsReceived() const { return m_packetsReceived; }
    uint32_t sessionId() const { return m_sessionId; }
    
    void setSessionId(uint32_t id) { m_sessionId = id; }
    void setAccountId(uint32_t id) { m_accountId = id; }
    
    bool hasInputPacketsQueued() const { return !m_inputPacketQueue.empty(); }
    void queueInputPacket(byte* data, uint32_t len);
    
    std::vector<InputPacket>& inputPacketQueue() { return m_inputPacketQueue; }
    
    void clearPacketQueues();
    
    void sendImmediateNoIncrement(const void* data, uint32_t len);
    
public:
    PacketTracker(IpAddress& addr, UdpSocket& socket);
    virtual ~PacketTracker();

    uint32_t accountId() const { return m_accountId; }

    void sendImmediate(const void* data, uint32_t len);
};

#endif//_EQP_PACKET_TRACKER_HPP_
