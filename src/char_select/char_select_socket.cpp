
#include "char_select_socket.hpp"

CharSelectSocket::CharSelectSocket(LogWriterCommon& logWriter)
: UdpSocket(logWriter)
{
    
}

CharSelectSocket::~CharSelectSocket()
{
    
}

ProtocolHandler* CharSelectSocket::createProtocolHandler(IpAddress& addr)
{
    return new CharSelectClient(addr, *this);
}

void CharSelectSocket::processPacketQueues()
{
    
}
