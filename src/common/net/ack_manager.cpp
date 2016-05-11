
#include "ack_manager.hpp"

AckManager::AckManager(IpAddress& addr, UdpSocket& socket)
: PacketTracker(addr, socket),
  m_ackExpected(0),
  m_ackLastReceived(65535),
  m_ackToSend(0)
{
    
}

AckManager::~AckManager()
{
    
}

AckManager::Sequence AckManager::compareSequence(uint16_t got, uint16_t expected)
{
	if (got == expected)
		return Sequence::Present;

	if ((got > expected && got < expected + WINDOW_SIZE) || got < (expected - WINDOW_SIZE))
		return Sequence::Future;

	return Sequence::Past;
}

void AckManager::checkSequencePacket(AlignedReader& r)
{
    uint16_t seq = toHostShort(r.uint16());
    
    switch (compareSequence(seq, m_ackExpected))
    {
    case Sequence::Present:
        m_ackExpected++;
        queueInputPacket(r.current(), r.remaining());
        break;
    
    default:
        break;
    }
}

void AckManager::checkSequenceFragment(AlignedReader& r)
{
    
}
