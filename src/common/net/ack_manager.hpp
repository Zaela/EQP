
#ifndef _EQP_ACK_MANAGER_HPP_
#define _EQP_ACK_MANAGER_HPP_

#include "define.hpp"
#include "netcode.hpp"
#include "packet_tracker.hpp"
#include "aligned.hpp"
#include <vector>

class AckManager : public PacketTracker
{
private:
    struct AckPacket
    {
        
    };
    
    enum Sequence
    {
        Present,
        Future,
        Past
    };
    
    static const uint16_t WINDOW_SIZE = 2048;

private:
    // These deliberately overflow
    uint16_t    m_ackExpected;
    uint16_t    m_ackLastReceived;
    uint16_t    m_ackToSend;

private:
    Sequence compareSequence(uint16_t got, uint16_t expected);

protected:
    void checkSequencePacket(AlignedReader& r);
    void checkSequenceFragment(AlignedReader& r);
    
public:
    AckManager(IpAddress& addr, UdpSocket& socket);
    virtual ~AckManager();
};

#endif//_EQP_ACK_MANAGER_HPP_
