
#ifndef _EQG_CHAR_SELECT_SOCKET_HPP_
#define _EQG_CHAR_SELECT_SOCKET_HPP_

#include "define.hpp"
#include "netcode.hpp"
#include "udp_socket.hpp"
#include "log_writer_common.hpp"
#include "protocol_handler.hpp"
#include "char_select_client.hpp"

class CharSelectSocket : public UdpSocket
{
private:
    virtual ProtocolHandler* createProtocolHandler(IpAddress& addr);

public:
    CharSelectSocket(LogWriterCommon& logWriter);
    virtual ~CharSelectSocket();

    void processPacketQueues();
};

#endif//_EQG_CHAR_SELECT_SOCKET_HPP_
