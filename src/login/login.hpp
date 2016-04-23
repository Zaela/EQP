
#ifndef _EQP_LOGIN_HPP_
#define _EQP_LOGIN_HPP_

#include "define.hpp"
#include "database.hpp"
#include "log_writer_common.hpp"
#include "ipc_buffer.hpp"
#include "exception.hpp"
#include "clock.hpp"
#include "eq_packet_protocol.hpp"
#include "login_crypto.hpp"
#include "aligned.hpp"
#include "source_id.hpp"
#include "server_op.hpp"
#include <vector>
#include <string>

#define EQP_LOGIN_PORT 5999
#define EQP_LOGIN_TIMEOUT_MILLISECONDS 60000

/*
struct PlayEverquestRequest_Struct
{
	uint16 Sequence;
	uint32 Unknown1;
	uint32 Unknown2;
	uint32 ServerNumber;
};

struct PlayEverquestResponse_Struct {
	uint8 Sequence;
	uint8 Unknown1[9];
	uint8 Allowed;
	uint16 Message;
	uint8 Unknown2[3];
	uint32 ServerNumber;
};
*/

class Login
{
private:
    struct Client
    {
        uint32_t ipAddress;
        uint16_t port;
        uint16_t recvAck;
        uint16_t sendAck;
        uint16_t progress;
        uint64_t lastActivityTimestamp;
    };
    
    enum Progress : uint16_t
    {
        None,
        Session,
        LoginRequested,
        LoggedIn,
        ReceivedServerList
    };
    
    static const int BUFFER_SIZE = 1024;
    
    enum LoginOp : uint16_t
    {
        LoginRequest        = 0x0001,
        LoginCredentials    = 0x0002,
        ServerListRequest   = 0x0004,
        PlayRequest         = 0x000d,
        EnterChat           = 0x000f,
        PollResponse        = 0x0011,
        ChatMessage         = 0x0016,
        LoginAccepted       = 0x0017,
        ServerListResponse  = 0x0018,
        PlayResponse        = 0x0021,
        Poll                = 0x0029,
    };
    
#pragma pack(1)
    struct AckPlus
    {
        uint16_t    combined;
        uint8_t     ackSize;
        uint16_t    ackOpcode;
        uint16_t    ackSeq;
        uint8_t     dataSize;
        uint16_t    dataProtoOpcode;
        uint16_t    dataSeq;
        uint16_t    dataOpcode;
        
        AckPlus(uint16_t clientSeq, uint16_t size, uint16_t serverSeq, uint16_t opcode)
        {
            // Ensure writes are aligned
            AlignedWriter w(this, sizeof(AckPlus));
            
            w.uint16(toNetworkShort(EQProtocol::Combined));
            w.uint8(0x04);
            w.uint16(toNetworkShort(EQProtocol::Ack));
            w.uint16(toNetworkShort(clientSeq));
            w.uint8(size - sizeof(AckPlus) + 0x06);
            w.uint16(toNetworkShort(EQProtocol::Packet));
            w.uint16(toNetworkShort(serverSeq));
            w.uint16(opcode);
        }
        
        AlignedWriter writer()
        {
            return AlignedWriter(((byte*)this) + sizeof(AckPlus), dataSize);
        }
    };
#pragma pack()

private:
    bool            m_shutdown;
    DatabaseThread  m_databaseThread;
    Database        m_database;
    LogWriterCommon m_logWriter;
    IpcRemote       m_ipc;

    bool            m_serverLocked;
    std::string     m_serverName;
    uint32_t        m_serverPlayerCount;

    std::vector<Client> m_clients;

    int     m_socket;
    byte    m_sockBuffer[BUFFER_SIZE];

    LoginCrypto m_crypto;

private:
    void initSocket();

    void processIpc(SharedRingBuffer::Packet& packet);
    void processProtocol(byte* data, int len, Client* client);
    void processCombined(byte* data, int len, Client* client);
    void processPacket(int len, IpAddress& addr);
    void processPacket(byte* data, int len, Client* client);
    void checkForTimeouts(uint64_t timestamp);
    void swapAndPop(Client* client);

    void sendSessionResponse(Client* client);
    void send(uint32_t ipAddress, uint16_t port, const void* data, uint32_t len);
    void send(Client* client, const void* data, uint32_t len);

    LoginCrypto& crypto() { return m_crypto; }
    
    void processLoginRequest(Client* client, uint16_t seq);
    void processCredentials(byte* data, int len, Client* client, uint16_t seq);
    void processServerListRequest(Client* client, uint16_t seq);

public:
    Login();
    ~Login();

    void init(const char* ipcPath);
    void mainLoop();
};

#endif//_EQP_LOGIN_HPP_
