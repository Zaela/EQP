
#ifndef _EQP_LOGIN_CLIENT_HPP_
#define _EQP_LOGIN_CLIENT_HPP_

#include "define.hpp"
#include "clock.hpp"

class LoginClient
{
public:
    enum Progress : uint16_t
    {
        None,
        Session,
        LoginRequested,
        LoggedIn,
        ReceivedServerList
    };

private:
    uint32_t    m_ipAddress;
    uint16_t    m_port;
    bool        m_isTrilogy;
    uint16_t    m_recvAck;
    uint16_t    m_sendAck;
    uint16_t    m_progress;
    uint16_t    m_playSequence;
    uint16_t    m_playAck;
    uint64_t    m_lastActivityTimestamp;
    int64_t     m_accountId;

public:
    LoginClient(uint32_t ip, uint16_t port);

    inline uint32_t ipAddress() const { return m_ipAddress; }
    inline uint16_t port() const { return m_port; }
    
    void flagAsTrilogy() { m_isTrilogy = true; }
    inline bool isTrilogy() const { return m_isTrilogy; }
    
    uint16_t recvAck() const { return m_recvAck; }
    uint16_t getRecvAckAndIncrement() { return m_recvAck++; }
    void setRecvAck(uint16_t val) { m_recvAck = val; }
    void incrementRecvAck() { m_recvAck++; }
    
    uint16_t getSendAckAndIncrement() { return m_sendAck++; }
    
    uint16_t progress() const { return m_progress; }
    void setProgress(uint16_t p) { m_progress = p; }
    
    uint16_t playSequence() const { return m_playSequence; }
    uint16_t playAck() const { return m_playAck; }
    void setPlayValues(uint16_t seq, uint16_t ack) { m_playSequence = seq; m_playAck = ack; }
    
    uint64_t lastActivityTime() const { return m_lastActivityTimestamp; }
    void setLastActivityTime() { m_lastActivityTimestamp = Clock::milliseconds(); }
    
    int64_t accountId() const { return m_accountId; }
    void setAccountId(int16_t id) { m_accountId = id; }
};

#endif//_EQP_LOGIN_CLIENT_HPP_
