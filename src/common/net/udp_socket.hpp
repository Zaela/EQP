
#ifndef _EQP_UDP_SOCKET_HPP_
#define _EQP_UDP_SOCKET_HPP_

#include "define.hpp"
#include "netcode.hpp"
#include "atomic_mutex.hpp"
#include "log_writer_common.hpp"
#include "protocol_handler.hpp"
#include <atomic>
#include <thread>
#include <vector>

class UdpSocket
{
protected:
    struct UdpClient
    {
        uint32_t            ipAddress;
        uint16_t            port;
        bool                hasInputPacketsQueued;
        bool                hasOutputPacketsQueued;
        int                 byteRateWritten; //decays every 20 milliseconds
        ProtocolHandler*    handler;
        
        UdpClient(IpAddress& addr, ProtocolHandler* protoHandler)
        : ipAddress(addr.sin_addr.s_addr),
          port(addr.sin_port),
          hasInputPacketsQueued(false),
          hasOutputPacketsQueued(false),
          byteRateWritten(0),
          handler(protoHandler)
        {
            
        }
        
        UdpClient(UdpClient&& o)
        {
            *this = std::move(o);
        }
        
        ~UdpClient()
        {
            if (handler)
            {
                delete handler;
                handler = nullptr;
            }
        }
        
        UdpClient& operator=(UdpClient&& o)
        {
            ipAddress               = o.ipAddress;
            port                    = o.port;
            hasInputPacketsQueued   = o.hasInputPacketsQueued;
            hasOutputPacketsQueued  = o.hasOutputPacketsQueued;
            byteRateWritten         = o.byteRateWritten;
            handler                 = o.handler;
            
            o.handler = nullptr;
            
            return *this;
        }
    };

public:
    struct Authorized
    {
        uint32_t    ipAddress;
        uint32_t    accountId;
        char        sessionKey[10];
    };
    
public:
    static const int BUFFER_SIZE = 1024;
    
protected:
    int     m_socket;
    byte    m_socketBuffer[BUFFER_SIZE];

    std::vector<UdpClient>  m_clients;
    //std::vector<UdpClient>  m_clientsPendingAuth;
    std::vector<Authorized> m_authorized;

    LogWriterCommon&        m_logWriter;

    byte    m_compressionBuffer[BUFFER_SIZE];
    byte    m_decompressionBuffer[BUFFER_SIZE];

private:
    virtual ProtocolHandler* createProtocolHandler(IpAddress& addr) = 0;

    void sendRaw(const void* data, uint32_t len, const IpAddress& addr);

public:
    UdpSocket(LogWriterCommon& logWriter);
    virtual ~UdpSocket();

    void open(uint16_t port);
    void close();
    void receive();
    
    void addClientAuth(Authorized& auth);
    void removeHandler(ProtocolHandler* handler);

    int getSocketFileDescriptor() const { return m_socket; }
    byte* getCompressionBuffer() { return m_compressionBuffer; }
    byte* getDecompressionBuffer() { return m_decompressionBuffer; }
};

#endif//_EQP_UDP_SOCKET_HPP_
