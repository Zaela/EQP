
#include "udp_socket.hpp"

UdpSocket::UdpSocket(LogWriterCommon& logWriter)
: m_socket(INVALID_SOCKET),
  m_logWriter(logWriter)
{
    
}

UdpSocket::~UdpSocket()
{
    close();
}

void UdpSocket::open(uint16_t port)
{
    m_socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    
    if (m_socket == INVALID_SOCKET)
        throw Exception("[UdpSocket::open] socket() failed");
    
    // Set non-blocking
#ifdef EQP_WINDOWS
    unsigned long nonblock = 1;
    if (::ioctlsocket(m_socket, FIONBIO, &nonblock))
#else
    if (::fcntl(m_socket, F_SETFL, O_NONBLOCK))
#endif
        throw Exception("[UdpSocket::open] Setting non-blocking mode failed");
    
    IpAddress addr;
    memset(&addr, 0, sizeof(IpAddress));
    
    addr.sin_family         = AF_INET;
    addr.sin_port           = toNetworkShort(port);
    addr.sin_addr.s_addr    = toNetworkLong(INADDR_ANY);
    
    if (::bind(m_socket, (struct sockaddr*)&addr, sizeof(IpAddress)))
        throw Exception("[UdpSocket::open] bind() failed");
    
    m_logWriter.log(Log::Info, "Listening for UDP packets on port %u", port);
}

void UdpSocket::close()
{
    if (m_socket != INVALID_SOCKET)
    {
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

void UdpSocket::addClientAuth(Authorized& auth)
{
    //check if the client is already in the pending auth queue
    m_authorized.push_back(auth);
}

void UdpSocket::receive()
{
    IpAddress addr;
    socklen_t addrLen = sizeof(IpAddress);
    
    for (;;)
    {
        int len = ::recvfrom(m_socket, (char*)m_socketBuffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addrLen);
            
        if (len <= 0)
        {
            int err = errno;
            if (err != EAGAIN)
                m_logWriter.log(Log::Error, "[UdpSocket::receive] recvfrom() failed, errno %i", err);
            return;
        }
        
        // Do we already know about this client?
        UdpClient* client   = nullptr;
        uint32_t ip         = addr.sin_addr.s_addr;
        uint16_t port       = addr.sin_port;
        
        for (UdpClient& cli : m_clients)
        {
            if (cli.ipAddress == ip && cli.port == port)
            {
                client = &cli;
                goto got_client;
            }
        }
        
        // If we reach here, this is a new client
        m_clients.emplace_back(addr, createProtocolHandler(addr));
        client = &m_clients.back();
        
    got_client:
        if (client->handler->receive(m_socketBuffer, (uint32_t)len))
            client->hasInputPacketsQueued = true;
    }
}

void UdpSocket::sendImmediate(const void* data, uint32_t len, const IpAddress& addr)
{
    // UDP sends are effectively instant from the application's point of view (no blocking/EAGAIN)
    // Furthermore, the only way they can really fail is if the OS's buffer overflows;
    // this is unlikely enough for us that we will simply ignore the possibility of failure
    ::sendto(m_socket, (const char*)data, (int)len, 0, (struct sockaddr*)&addr, sizeof(IpAddress));
}

void UdpSocket::removeHandler(ProtocolHandler* handler)
{
    uint32_t n = m_clients.size();
    
    for (uint32_t i = 0; i < n; i++)
    {
        UdpClient& cli = m_clients[i];
        
        if (cli.handler == handler)
        {
            delete cli.handler;
            cli.handler = nullptr;
            // Swap and pop
            if (i < (n - 1))
            {
                m_clients[i] = std::move(m_clients.back());
                m_clients.pop_back();
            }
            
            return;
        }
    }
}
