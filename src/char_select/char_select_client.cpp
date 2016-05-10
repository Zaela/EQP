
#include "char_select_client.hpp"
#include "udp_socket.hpp"

CharSelectClient::CharSelectClient(IpAddress& addr, UdpSocket& socket)
: ProtocolHandler(addr, socket)
{
    
}

CharSelectClient::~CharSelectClient()
{
    
}
