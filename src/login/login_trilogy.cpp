
#include "login.hpp"

bool Login::isTrilogyClient(LoginClient* client, byte* data, int len)
{
#pragma pack(1)
    struct VersionRequest
    {
        uint16_t    header;
        uint16_t    sequence;
        uint16_t    ackRequest;
        uint16_t    counter;
        uint16_t    opcode;
        uint32_t    crc;
    };
#pragma pack()
    
    printf("-> ");
    for (int i = 0; i < len; i++) printf("%02x ", data[i]);
    printf("\n");
    
    // Everything before crc should be aligned
    VersionRequest* vr = (VersionRequest*)data;
    
    if (vr->header != (HasAckRequest | HasCounter | IsFirstPacket) || vr->opcode != TrilogyOp::Version)
        return false;
    
    AlignedReader r(vr, sizeof(VersionRequest));
    r.advance(offsetof(VersionRequest, crc));
    
    if (r.uint32() != CRC::calc32NetworkByteOrder(vr, offsetof(VersionRequest, crc)))
        return false;
    
    client->flagAsTrilogy();
    client->setRecvAck(Random::uint16());
    
    printf("%i vs %lu\n", len, sizeof(VersionRequest));
    printf("opcode %04x\n", vr->opcode);
    printf("crc: %08x vs %08x\n", vr->crc, CRC::calc32NetworkByteOrder(vr, offsetof(VersionRequest, crc)));
    
#pragma pack(1)
    struct VersionReply : public TrilogyFullHeader
    {
        char        version[sizeof(EQP_LOGIN_VERSION_TRILOGY)];
        uint32_t    crc;
        
        VersionReply(LoginClient* client, uint16_t op, uint16_t ackRequest)
        : TrilogyFullHeader(client, op, ackRequest, 0, (HasAckRequest | HasCounter | IsFirstPacket | HasAckResponse)) { }
    };
#pragma pack()
    
    VersionReply reply(client, TrilogyOp::Version, vr->ackRequest);
    AlignedWriter w(&reply, sizeof(reply));
    
    w.advance(sizeof(TrilogyFullHeader));
    
    // version
    w.string(EQP_LOGIN_VERSION_TRILOGY, sizeof(EQP_LOGIN_VERSION_TRILOGY)); // sizeof will include null terminator
    // crc
    w.uint32(CRC::calc32NetworkByteOrder(&reply, w.position()));
    
    printf("<- ");
    for (int i = 0; i < sizeof(reply); i++) printf("%02x ", ((byte*)&reply)[i]);
    printf("\n");
    
    send(client, &reply, sizeof(reply));
    
    client->setProgress(TrilogyProgress::VersionSent);
    return true;
}

void Login::processProtocolTrilogy(byte* data, int len, LoginClient* client)
{
    printf("-> ");
    for (int i = 0; i < len; i++) printf("%02x ", data[i]);
    printf("\n");
    
    AlignedReader r(data, len);
    
    uint16_t header     = r.uint16();
    uint16_t ackRequest = 0;
    
    // sequence - always present
    r.advance(sizeof(uint16_t));
    
    if (header & HasAckResponse)
    {
        if (toHostShort(r.uint16()) == client->recvAck())
            client->incrementRecvAck();
        
        if (len == 10)
            return;
    }
    
    if (header & HasAckRequest)
        ackRequest = r.uint16();
    
    if (header & HasCounter)
        r.advance(sizeof(uint8_t) * 2);
    
    uint16_t opcode = r.uint16();
    
    switch (opcode)
    {
    case TrilogyOp::TrilogyCredentials:
        processCredentialsTrilogy(client, r, ackRequest);
        break;
        
    default:
        printf("opcode: %04x\n", opcode);
        sendTrilogyErrorMessage(client, "Whoops!", sizeof("Whoops!"), ackRequest);
        break;
    }
}

void Login::processCredentialsTrilogy(LoginClient* client, AlignedReader& r, uint16_t ackRequest)
{
#pragma pack(1)
    struct Credentials
    {
        char username[20];
        char password[20];
    };
    
    struct Session : public TrilogyFullHeader
    {
        char        sessionId[10];
        char        unused[7];
        uint32_t    unknown;
        uint32_t    crc;
        
        Session(LoginClient* client, uint16_t op, uint16_t ackRequest)
        : TrilogyFullHeader(client, op, ackRequest, 1) { }
    };
#pragma pack()
    
    // The client spams this packet faster than we can check the DB and reply...
    uint16_t progress = client->progress();
    if ((progress != TrilogyProgress::VersionSent && progress != LoginClient::Progress::LoggedIn) || r.remaining() < sizeof(Credentials))
        return;
    
    crypto().decrypt(r.current(), sizeof(Credentials), true);
    
    Credentials* cred = (Credentials*)crypto().data();
    
    printf("account: %s, password: %s\n", cred->username, cred->password);
    
    // Need to copy password as hash() will be clobbering the crypto buffer below
    char passcopy[20];
    uint32_t passlen = strlen(cred->password);
    
    if (passlen > 19)
        return;
    
    memcpy(passcopy, cred->password, 20);
    
    Query query;
    m_database.prepare(query, "SELECT rowid, password, salt FROM local_login WHERE username = ?");
    query.bindString(1, cred->username, strlen(cred->username), false);
    query.executeSynchronus();
    
    while (query.select())
    {
        uint32_t plen;
        uint32_t slen;
        int64_t id          = query.getInt64(1);
        const byte* pdata   = query.getBlob(2, plen);
        const byte* salt    = query.getBlob(3, slen);
        
        crypto().hash(passcopy, passlen, salt, slen);
        
        if (memcmp(crypto().data(), pdata, plen) == 0)
        {
            // Success
            crypto().clear();
            memset(cred, 0, sizeof(Credentials));
            
            Session reply(client, TrilogyOp::TrilogySession, ackRequest);
            AlignedWriter w(&reply, sizeof(reply));
            
            w.advance(sizeof(TrilogyFullHeader));
            
            // sessionId
            snprintf(reply.sessionId, sizeof(reply.sessionId), "LS#%u", (uint32_t)id);
            // "unused"
            w.advance(sizeof(reply.sessionId));
            w.string("unused", sizeof("unused"));
            // unknown
            w.uint32(4);
            // crc
            w.uint32(CRC::calc32NetworkByteOrder(&reply, w.position()));
            
            printf("<- ");
            for (int i = 0; i < sizeof(reply); i++) printf("%02x ", ((byte*)&reply)[i]);
            printf("\n");
            
            send(client, &reply, sizeof(reply));
            client->setProgress(LoginClient::Progress::LoggedIn);
            return;
        }
    }

    crypto().clear();
    memset(cred, 0, sizeof(Credentials));
    
    sendTrilogyErrorMessage(client, "Whoops!", sizeof("Whoops!"), ackRequest);
}

void Login::sendTrilogyErrorMessage(LoginClient* client, const char* msg, uint32_t length, uint16_t ackRequest)
{
#pragma pack(1)
    struct Header
    {
        uint16_t    header;
        uint16_t    sequence;
    };
#pragma pack()
    
    byte buffer[1024];
    AlignedWriter w(buffer, sizeof(buffer));
    
    // header
    w.uint16(ackRequest ? HasAckResponse : 0);
    // sequence
    w.uint16(toNetworkShort(client->getSendAckAndIncrement()));
    // ackResponse
    if (ackRequest)
        w.uint16(ackRequest);
    // opcode
    w.uint16(TrilogyOp::TrilogyError);
    // "error" string
    w.string("Error: ", 7);
    // msg
    w.string(msg, length);
    // crc
    w.uint32(CRC::calc32NetworkByteOrder(buffer, w.position()));
    
    printf("<- ");
    for (uint32_t i = 0; i < w.position(); i++) printf("%02x ", buffer[i]);
    printf("\n");
    
    send(client, buffer, w.position());
}
