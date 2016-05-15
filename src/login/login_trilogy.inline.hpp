
#define EQP_LOGIN_VERSION_TRILOGY "8-09-2001 14:25"

private:
    enum TrilogyOp : uint16_t
    {
        TrilogyCredentials  = 0x0001,
        TrilogyError        = 0x0002,
        TrilogySession      = 0x0004,
        Banner              = 0x0052,
        Version             = 0x0059
    };
    
    enum TrilogyHeader
    {
        UnknownBit0         = 0x0001,
        HasAckRequest       = 0x0002,
        IsClosing           = 0x0004,
        IsFragment          = 0x0008,
        HasCounter          = 0x0010,
        IsFirstPacket       = 0x0020,
        IsClosing2          = 0x0040,
        IsSequenceEnd       = 0x0080,
        IsSpecialAckRequest = 0x0100,
        UnknownBit9         = 0x0200,
        HasAckResponse      = 0x0400
    };
    
    enum TrilogyProgress : uint16_t
    {
        None,
        VersionSent = 1000
    };

#pragma pack(1)
    struct TrilogyFullHeader
    {
        uint16_t    header;
        uint16_t    sequence;
        uint16_t    ackResponse;
        uint16_t    ackRequest;
        uint8_t     counterHigh;
        uint8_t     counterLow;
        uint16_t    opcode;
        
        TrilogyFullHeader(LoginClient* client, uint16_t op, uint16_t ackRequest, uint8_t counter, uint16_t headerValue = (HasAckRequest | HasCounter | HasAckResponse))
        : header(headerValue),
          sequence(toNetworkShort(client->getSendAckAndIncrement())),
          ackResponse(ackRequest),
          ackRequest(toNetworkShort(client->recvAck())),
          counterHigh(1),
          counterLow(counter),
          opcode(op)
        {
            
        }
    };
#pragma pack()

public:
    bool isTrilogyClient(LoginClient* client, byte* data, int len);

    void processProtocolTrilogy(byte* data, int len, LoginClient* client);
    void processCredentialsTrilogy(LoginClient* client, AlignedReader& r, uint16_t ackRequest);

    void sendTrilogyErrorMessage(LoginClient* client, const char* msg, uint32_t length, uint16_t ackRequest = 0);
