
#ifndef _EQP_CHAR_SELECT_CLIENT_HPP_
#define _EQP_CHAR_SELECT_CLIENT_HPP_

#include "define.hpp"
#include "netcode.hpp"
#include "protocol_handler.hpp"

class UdpSocket;

class CharSelectClient : public ProtocolHandler
{
private:
    
public:
    CharSelectClient(IpAddress& addr, UdpSocket& socket);
    virtual ~CharSelectClient();
};

#endif//_EQP_CHAR_SELECT_CLIENT_HPP_
