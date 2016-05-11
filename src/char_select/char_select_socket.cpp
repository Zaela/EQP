
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
    for (UdpClient& cli : m_clients)
    {
        if (cli.hasInputPacketsQueued)
        {
            ((CharSelectClient*)cli.handler)->processInputPacketQueue();
            cli.hasInputPacketsQueued = false;
        }
        
        if (cli.hasOutputPacketsQueued)
        {
            if (((CharSelectClient*)cli.handler)->processOutputPacketQueue())
                cli.hasOutputPacketsQueued = false;
        }
    }
}
