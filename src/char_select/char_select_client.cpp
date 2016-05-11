
#include "char_select_client.hpp"
#include "udp_socket.hpp"

CharSelectClient::CharSelectClient(IpAddress& addr, UdpSocket& socket)
: ProtocolHandler(addr, socket)
{
    
}

CharSelectClient::~CharSelectClient()
{
    
}

void CharSelectClient::processInputPacketQueue()
{
    auto& queue = inputPacketQueue();
    
    for (InputPacket& p : queue)
    {
        AlignedReader r(p.data, p.length);
        processInputPacket(r);
    }
    
    queue.clear();
}

bool CharSelectClient::processOutputPacketQueue()
{
    return true;
}

void CharSelectClient::processInputPacket(AlignedReader& r)
{
    // Opcodes with a low-order byte of 0 (e.g. 0x4200 -> 00 42 in memory)
    // are preceded with an extra 0 byte (e.g. 00 42 -> 00 00 42)
    // We correct this by skipping the first byte if it's 0
    if (r.peekByte() == 0)
        r.advance(sizeof(byte));
    
    uint16_t opcode = r.uint16();
    
redo:
    switch (m_expansion)
    {
    case Expansion::Ordinal::Titanium:
        //handlePacketTitanium(r);
        break;
    
    case Expansion::Ordinal::Unknown:
        // We figure out what expansion we are by examining the first packet we receive with an application opcode
        if (determineExpansion(opcode))
            goto redo;
        break;
        
    default:
        break;
    }
}

bool CharSelectClient::determineExpansion(uint16_t opcode)
{
    Expansion::Ordinal expansion;
    
    switch (opcode)
    {
#ifndef EQP_DISABLE_TITANIUM
    case Titanium::Op::SendLoginInfo:
        expansion = Expansion::Ordinal::Titanium;
        break;
#endif
        
    default:
        goto not_found;
    }
    
    m_expansion = expansion;
    return true;

not_found:
    // If we reach here, reject the client
    disconnect();
    return false;
}
