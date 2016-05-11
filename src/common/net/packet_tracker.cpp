
#include "packet_tracker.hpp"
#include "udp_socket.hpp"

PacketTracker::PacketTracker(IpAddress& addr, UdpSocket& socket)
: m_socketFd(socket.getSocketFileDescriptor()),
  m_address(addr),
  m_socket(socket),
  m_packetsSent(0),
  m_packetsReceived(0),
  m_sessionId(0)
{

}

PacketTracker::~PacketTracker()
{
    
}

void PacketTracker::queueInputPacket(byte* data, uint32_t len)
{
    byte* copy = new byte[len];
    memcpy(copy, data, len);
    
    m_inputPacketQueue.emplace_back(copy, len);
}

void PacketTracker::clearPacketQueues()
{
    m_inputPacketQueue.clear();
}

void PacketTracker::sendImmediateNoIncrement(const void* data, uint32_t len)
{
    const byte* d = (const byte*)data;
    printf("-> ");
    for (uint32_t i = 0; i < len; i++) printf("%02x ", d[i]);
    printf("\n");
    // This function uses sendto() directly to avoid needing to dereference the UdpSocket just to get the fd
    
    // UDP sends are effectively instant from the application's point of view (no blocking/EAGAIN)
    // Furthermore, the only way they can really fail is if the OS's buffer overflows;
    // this is unlikely enough for us that we will simply ignore the possibility of failure
    ::sendto(m_socketFd, (const char*)data, (int)len, 0, (struct sockaddr*)&m_address, sizeof(IpAddress));
}

void PacketTracker::sendImmediate(const void* data, uint32_t len)
{
    incrementPacketsSent();
    sendImmediateNoIncrement(data, len);
}
