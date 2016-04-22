
#include "login.hpp"

/*
** Basic login process ("->" is the client, "<-" is the server)
**
** -> SessionRequest
** <- SessionResponse
** -> LoginRequest
** <- ChatMessage (containing worthless data, including... "ChatMessage")
** -> LoginCredentials
** [If credentials are bad]
** <- LoginAccepted with failure data
** [If credentials are good]
** <- LoginAccepted with success data, some of which is encrypted
** -> ServerListRequest
** <- ServerListResponse
**
** [While idling at server select]
** Client periodically re-sends ServerListReqest and expects an updated ServerListResponse
*/

Login::Login()
: m_shutdown(false),
  m_databaseThread(m_logWriter),
  m_database(m_databaseThread, m_logWriter),
  m_logWriter(SourceId::Login, m_ipc),
  m_socket(INVALID_SOCKET)
{
    
}

Login::~Login()
{
    if (m_socket != INVALID_SOCKET)
        closesocket(m_socket);
}

void Login::init(const char* ipcPath)
{
    // Not initializing the DatabaseThread since we aren't actually using background
    // query processing; not point in being too efficient, may as well keep everything
    // single-threaded.
    m_database.init(EQP_SQLITE_MAIN_DATABASE_PATH, EQP_SQLITE_MAIN_SCHEMA_PATH);
    
    // Shared Memory IPC area
    (void)ipcPath;
    //m_ipc.init(ipcPath);
    
    initSocket();
}

void Login::initSocket()
{
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (m_socket == INVALID_SOCKET)
        throw Exception("[Login::initSocket] socket() failed");
    
    // Set non-blocking
#ifdef EQP_WINDOWS
    unsigned long nonblock = 1;
    if (ioctlsocket(m_socket, FIONBIO, &nonblock))
#else
    if (fcntl(m_socket, F_SETFL, O_NONBLOCK))
#endif
        throw Exception("[Login::initSocket] Setting non-blocking mode failed");
    
    IpAddress addr;
    memset(&addr, 0, sizeof(IpAddress));
    
    addr.sin_family         = AF_INET;
    addr.sin_port           = toNetworkShort(EQP_LOGIN_PORT);
    addr.sin_addr.s_addr    = toNetworkLong(INADDR_ANY);
    
    if (bind(m_socket, (struct sockaddr*)&addr, sizeof(IpAddress)))
        throw Exception("[Login::initSocket] bind() failed");
}

void Login::mainLoop()
{
    uint64_t timestamp  = Clock::milliseconds();
    uint64_t timecount  = 0;
    socklen_t addrLen   = sizeof(IpAddress);
    
    IpAddress addr;
    
    for (;;)
    {
        // Check IPC input
        /*for (;;)
        {
            SharedRingBuffer::Packet packet;
            
            if (!m_ipc.pop(packet))
                break;
            
            processIpc(packet);
        }*/
        
        // Check socket input
        for (;;)
        {
            int len = ::recvfrom(m_socket, (char*)m_sockBuffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addrLen);
            
            if (len == 0)
                break;
            
            if (len < 0)
            {
                m_logWriter.log(Log::Error, "[Login::mainLoop] recvfrom() failed");
                break;
            }
            
            processPacket(len, addr);
        }
        
        // Every 10 seconds, check for timed out clients
        uint64_t time = Clock::milliseconds();
        timecount += timestamp - time;
        timestamp = time;
        
        if (timecount >= 10000)
        {
            timecount = 0;
            checkForTimeouts(timestamp);
        }
        
        if (m_shutdown)
            return;
        
        Clock::sleepMilliseconds(50);
    }
}

void Login::processIpc(SharedRingBuffer::Packet& packet)
{
    // There are only a very small number of opcodes that login cares about
    switch (packet.opcode())
    {
    case ServerOp::Shutdown:
        m_shutdown = true;
        m_logWriter.log(Log::Info, "Received shutdown command from Master");
        break;
    default:
        break;
    }
}

void Login::processPacket(int len, IpAddress& addr)
{
    uint32_t ip     = addr.sin_addr.s_addr;
    uint16_t port   = addr.sin_port;
    Client* client  = nullptr;
    
    // Find the Client in our list
    uint32_t i = 0;
    for (; i < m_clients.size(); i++)
    {
        Client& cli = m_clients[i];
        
        if (cli.ipAddress == ip && cli.port == port)
        {
            client = &cli;
            break;
        }
    }
    
    // Is it a new client? If so, add them
    if (!client)
    {
        Client cli;
        
        cli.ipAddress   = ip;
        cli.port        = port;
        cli.recvAck     = 0;
        cli.sendAck     = 0;
        cli.progress    = 0;
        
        m_clients.push_back(cli);
        client = &m_clients.back();
    }
    
    client->lastActivityTimestamp = Clock::milliseconds();
    
    // Anything meaningful will be at least protocol opcode + sequence, 4 bytes
    if (len < 4)
        return;
    
    processProtocol(m_sockBuffer, len, client);
}

void Login::processProtocol(byte* data, int len, Client* client)
{
    uint16_t opcode = toHostShort(*(uint16_t*)data);
    data += 2;
    len -= 2;
    
    switch (opcode)
    {
    case EQProtocol::SessionRequest:
        client->progress = 1;
        sendSessionResponse(client);
        break;
    case EQProtocol::Combined:
        processCombined(data, len, client);
        break;
    case EQProtocol::Packet:
        processPacket(data, len, client);
        break;
    case EQProtocol::SessionDisconnect:
        swapAndPop(client);
        break;
    default:
        break;
    }
}

void Login::processCombined(byte* data, int len, Client* client)
{
    int read = 0;
    
    while (read < len)
    {
        int size = data[read];
        read++;
        
        if ((read + size) > len)
            break;
        
        processProtocol(data + read, size, client);
        
        read += size;
    }
}

void Login::processPacket(byte* data, int len, Client* client)
{
    uint16_t seq    = toHostShort(*(uint16_t*)data);
    uint16_t opcode = *(uint16_t*)(data + sizeof(uint16_t));
    
    data += sizeof(uint16_t) * 2;
    
    switch (opcode)
    {
    case LoginRequest:
    {
#pragma pack(1)
        struct Reply
        {
            AckPlus ackPlus;
            byte    data[16];
            byte    message[12];
            
            Reply(uint16_t clientSeq, uint8_t dataSize, uint16_t serverSeq, uint16_t opcode)
            : ackPlus(clientSeq, dataSize, serverSeq, opcode) { }
        };
#pragma pack()
        
        if (client->progress != 1)
            break;
        
        Reply reply(seq, sizeof(Reply), client->sendAck++, ChatMessage);
        
        memset(reply.data, 0, sizeof(reply.data));
        
        reply.data[ 0] = 0x02;
        reply.data[10] = 0x01;
        reply.data[11] = 0x65;
        
        memcpy(reply.message, "ChatMessage", sizeof(reply.message));
        
        send(client, &reply, sizeof(reply));
        
        client->progress = 2;
        break;
    }
    
    case LoginCredentials:
        processCredentials(data, len, client, seq);
        break;
    
    case ServerListRequest:
    {
#pragma pack(1)
        struct ServerList
        {
            AckPlus ackPlus;
            //other fields...
        };
#pragma pack()
        
        if (client->progress != 3)
            break;
        break;
    }
    
    default:
        break;
    }
}

void Login::checkForTimeouts(uint64_t timestamp)
{
    uint32_t i = 0;
    
    while (i < m_clients.size())
    {
        Client& cli = m_clients[i];
        
        if ((timestamp - cli.lastActivityTimestamp) > EQP_LOGIN_TIMEOUT_MILLISECONDS)
        {
            swapAndPop(&cli);
            continue;
        }
        
        i++;
    }
}

void Login::swapAndPop(Client* client)
{
    uint32_t i = 0;
    uint32_t n = m_clients.size() - 1;
    
    while (i < n)
    {
        if (&m_clients[i] == client)
        {
            m_clients[i] = m_clients[n];
            break;
        }
        
        i++;
    }
    
    m_clients.pop_back();
}
/*
void Login::sendAck(uint16_t seq, Client* client)
{
#pragma pack(1)
    struct Ack
    {
        uint16_t opcode;
        uint16_t seq;
    };
#pragma pack()
    
    Ack ack;
    ack.opcode  = toNetworkShort(EQProtocol::Ack);
    ack.seq     = toNetworkShort(seq);
    
    send(client, &ack, sizeof(Ack));
}
*/
void Login::sendSessionResponse(Client* client)
{
#pragma pack(1)
    struct SessionRequest
    {
        uint16_t    opcode;
        uint32_t    unknown;
        uint32_t    session;
        uint32_t    maxLength;
    };
    
    struct SessionResponse
    {
        uint16_t    opcode;
        uint32_t    session;
        uint32_t    key;
        uint8_t     validation;
        uint8_t     format;
        uint8_t     unknownA;
        uint32_t    maxLength;
        uint32_t    unknownB;
    };
#pragma pack()
    
    SessionRequest* req = (SessionRequest*)m_sockBuffer;
    
    SessionResponse resp;
    memset(&resp, 0, sizeof(SessionResponse));
    
    resp.opcode     = toNetworkShort(EQProtocol::SessionResponse);
    resp.session    = req->session;
    resp.maxLength  = req->maxLength;
    
    send(client, &resp, sizeof(SessionResponse));
}

void Login::send(uint32_t ipAddress, uint16_t port, const void* data, uint32_t len)
{
    IpAddress addr;
    
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = ipAddress;
    addr.sin_port           = port;
    
    // Not many ways this can visibly fail, and we don't really care if it does
    ::sendto(m_socket, (const char*)data, (int)len, 0, (struct sockaddr*)&addr, sizeof(IpAddress));
}

void Login::send(Client* client, const void* data, uint32_t len)
{
    send(client->ipAddress, client->port, data, len);
}

void Login::processCredentials(byte* data, int len, Client* client, uint16_t seq)
{
    // Must be at least 10 + encrypted portion (min 8)
    if (len < 18 || (uint32_t)len > BUFFER_SIZE || client->progress != 2)
        return;
    
    data += 10;
    len -= 10;
    
    crypto().decrypt(data, len);
    
    const char* username    = (const char*)crypto().data();
    uint32_t namelen        = strlen(username);
    const char* password    = (const char*)crypto().data() + namelen + 1;
    uint32_t passlen        = strlen(password);
    
    if (passlen >= 64)
        return;
    
    // Need to copy password since hash() will be clobbering it below
    char passcopy[64];
    memcpy(passcopy, password, passlen);
    
    Query query;
    m_database.prepare(query, "SELECT rowid, password, salt FROM local_login WHERE username = ?");
    query.bindString(1, username, namelen, false);
    query.executeSynchronus();
    
    int64_t id = 0;
    
#pragma pack(1)
    struct Request
    {
        uint16_t    unknown[5];
        byte        data[16];
    };
    
    struct Accepted
    {
        AckPlus     ackPlus;
        uint16_t    unknown[5];
        byte        encrypted[80];
        
        Accepted(uint16_t clientSeq, uint16_t dataSize, uint16_t serverSeq, uint16_t opcode)
        : ackPlus(clientSeq, dataSize, serverSeq, opcode) { }
    };
    
    struct Rejected
    {
        AckPlus     ackPlus;
        uint16_t    unknown[5];
        uint64_t    data[5];
        byte        lastData;
        
        Rejected(uint16_t clientSeq, uint16_t dataSize, uint16_t serverSeq, uint16_t opcode)
        : ackPlus(clientSeq, dataSize, serverSeq, opcode) { }
    };
    
    struct Attempts
    {
        uint64_t    unknown;
        uint32_t    loginId;
        char        key[11]; // 11th byte is a null terminator
        uint32_t    count;
        uint32_t    data[12];
    };
#pragma pack()
    
    while (query.select())
    {
        uint32_t plen;
        uint32_t slen;
        id                  = query.getInt64(1);
        const byte* pdata   = query.getBlob(2, plen);
        const byte* salt    = query.getBlob(3, slen);
        
        crypto().hash(passcopy, passlen, salt, slen);
        
        if (memcmp(crypto().data(), pdata, plen) == 0)
        {
            // Success
            goto login;
        }
        else
        {
            // Failure
            crypto().clear();
            
            // Send LoginAccepted failure packet
            Rejected rejected(seq, sizeof(Rejected), client->sendAck++, LoginAccepted);
            
            rejected.unknown[0] = 0x0003;
            rejected.unknown[1] = 0x0000;
            rejected.unknown[2] = 0x0200;
            rejected.unknown[3] = 0x0000;
            rejected.unknown[4] = 0x0000;
            
            rejected.data[0] = 0x9f803c647359769b;
            rejected.data[1] = 0xb6ee57f179a7041a;
            rejected.data[2] = 0x9f803c648ca68964;
            rejected.data[3] = 0xb6ee57f179a7041a;
            rejected.data[4] = 0x9f803c648ca68964;
            
            rejected.lastData = 0x1a;
            
            send(client, &rejected, sizeof(Rejected));
            return;
        }
    }
    
    // If we reach here, there was no entry in the database for the given username
    // To maximize laziness for both us and the user, create a new local login account from the provided input
    
    m_database.prepare(query, "INSERT INTO local_login (username, password, salt) VALUES (?, ?, ?)");
    
    // We haven't clobbered anything if the select above hasn't happened
    query.bindString(1, username, namelen); // username gets copied internally here, so okay to clobber below
    
    byte salt[16]; // 128-bit salt
    sqlite3_randomness(sizeof(salt), salt);
    
    crypto().hash(passcopy, passlen, salt, sizeof(salt));
    
    query.bindBlob(2, crypto().data(), crypto().hashSize(), false);
    query.bindBlob(3, salt, sizeof(salt), false);
    
    query.executeSynchronus();
    
    id = query.getLastInsertId();
    
login:
    // Zero out the crypto buffer before sending anything to the client
    crypto().clear();
    
    // Send LoginAccepted success packet
    const Request* req = (Request*)data;
    
    Accepted accepted(seq, sizeof(Accepted), client->sendAck++, LoginAccepted);
    
    accepted.unknown[0] = req->unknown[0];
    accepted.unknown[1] = req->unknown[1];
    accepted.unknown[2] = req->unknown[2];
    accepted.unknown[3] = req->unknown[3];
    accepted.unknown[4] = req->unknown[4];
    
    Attempts attempts;
    
    attempts.unknown    = 0x0000000000000001ULL;
    attempts.loginId    = (uint32_t)id;
    memcpy(attempts.key, "0000000000", sizeof(attempts.key)); // Don't bother with a real session key
    //fixme: everything below here is an unaligned write
    attempts.count      = 0;
    
    attempts.data[ 0] = 0x03ffffff; // The ff's suppress the "Vote Now!" dialog on reaching server select
    attempts.data[ 1] = 0x02000000;
    attempts.data[ 2] = 0x000003e7;
    attempts.data[ 3] = 0xffffffff;
    attempts.data[ 4] = 0x000005a0;
    attempts.data[ 5] = 0x02000000;
    attempts.data[ 6] = 0x000003ff;
    attempts.data[ 7] = 0x00000000;
    attempts.data[ 8] = 0x00000063;
    attempts.data[ 9] = 0x00000001;
    attempts.data[10] = 0x00000000;
    attempts.data[11] = 0x00000000;
    
    // Encrypt and copy into Accepted
    crypto().encrypt(&attempts, sizeof(attempts));
    memcpy(accepted.encrypted, crypto().data(), sizeof(accepted.encrypted));
    
    send(client, &accepted, sizeof(accepted));
    
    client->progress = 3;
}
