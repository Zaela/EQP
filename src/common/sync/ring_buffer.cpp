
#include "ring_buffer.hpp"

void SharedRingBuffer::init(uint32_t totalSize)
{
    m_maxOffset = totalSize - offsetof(SharedRingBuffer, m_memoryRegion);
    
    m_startOffset.store(0);
    m_endOffset.store(0);
    m_retreatOffset.store(0);
}

uint32_t SharedRingBuffer::calcPacketLength(uint32_t len)
{
    uint32_t totalLen   = len + PACKET_OVERHEAD;
    uint32_t rem        = len % 8;
    
    // Make sure the following packet will be nicely aligned
    // Using 8-byte boundaries rather than 16 because we'll never use long doubles
    if (rem)
        totalLen += 8 - rem;
    
    return totalLen;
}

bool SharedRingBuffer::pop(SharedRingBuffer::Packet& out)
{
    uint32_t endOffset      = m_endOffset.load();
    uint32_t startOffset    = m_startOffset.load();
    
    if (endOffset == startOffset)
        return false;
    
    // If the endOffset is lower, the producer has restarted from the beginning of the memory region
    if (endOffset < startOffset)
    {
        endOffset = m_retreatOffset.load();
        
        if (startOffset >= endOffset)
        {
            m_retreatOffset.store(0);
            startOffset = 0;
        }
    }
    
    InternalPacket* p = (InternalPacket*)&m_memoryRegion[startOffset];
    
    out.set(p->opcode(), p->sourceId(), p->length(), p->data());
    
    // Must not happen until we are done with the shared data!
    m_startOffset.store(startOffset + p->totalLength());
    
    return true;
}

bool SharedRingBuffer::push(ServerOp opcode, int sourceId, uint32_t len, const byte* data)
{
    uint32_t startOffset    = m_startOffset.load();
    uint32_t endOffset      = m_endOffset.load();
    uint32_t totalLen       = calcPacketLength(len);
    
    // Check if there is sufficient spare at the end of the memory region
    // If endOffset is lower, then we only have up to startOffset available for use
    if (endOffset < startOffset)
    {
        // Minus 1 is needed to avoid an ambiguous situation when startOffset == endOffset
        if ((startOffset - endOffset - 1) < totalLen)
            return false;
    }
    else
    {
        if ((m_maxOffset - endOffset) < totalLen)
        {
            // Try to restart from the beginning of the memory region
            if (startOffset <= totalLen)
                return false;
            
            m_retreatOffset.store(endOffset);
            endOffset = 0;
        }
    }
    
    // We have space: copy the data + header into the available space starting at endOffset
    InternalPacket* packet = (InternalPacket*)&m_memoryRegion[endOffset];
    
    packet->m_opcode        = opcode;
    packet->m_sourceId      = sourceId;
    packet->m_length        = len;
    packet->m_totalLength   = totalLen;
    if (data && len)
        memcpy(packet->m_data, data, len);
    
    // We are done writing the data, allow the consumer to read it from this moment forward
    m_endOffset.store(endOffset + totalLen);
    
    return true;
}

SharedRingBuffer::Packet::Packet()
: m_opcode(ServerOp::None),
  m_sourceId(0),
  m_length(0),
  m_data(nullptr)
{

}

SharedRingBuffer::Packet::Packet(ServerOp opcode, int sourceId, uint32_t len, const byte* data)
: m_opcode(opcode),
  m_sourceId(sourceId),
  m_length(len),
  m_data(nullptr)
{
    if (data && len)
    {
        m_data = new byte[len];
        memcpy(m_data, data, len);
    }
}

SharedRingBuffer::Packet::Packet(Packet&& o)
: m_data(nullptr)
{
    *this = std::move(o);
}

SharedRingBuffer::Packet::~Packet()
{
    if (m_data)
    {
        delete[] m_data;
        m_data = nullptr;
    }
}

SharedRingBuffer::Packet& SharedRingBuffer::Packet::operator=(SharedRingBuffer::Packet&& o)
{
    if (m_data)
    {
        delete[] m_data;
        m_data = nullptr;
    }
    
    m_opcode    = o.m_opcode;
    m_sourceId  = o.m_sourceId;
    m_length    = o.m_length;
    
    if (o.m_data)
        m_data = o.m_data;
    
    o.m_length  = 0;
    o.m_data    = nullptr;
    
    return *this;
}

void SharedRingBuffer::Packet::set(ServerOp opcode, int sourceId, uint32_t len, const byte* data)
{
    if (m_data)
    {
        delete[] m_data;
        m_data = nullptr;
    }
    
    m_opcode    = opcode;
    m_sourceId  = sourceId;
    m_length    = len;
    
    if (data && len)
    {
        m_data = new byte[len];
        memcpy(m_data, data, len);
    }
}

byte* SharedRingBuffer::Packet::takeOwnershipOfData()
{
    byte* ret   = m_data;
    m_data      = nullptr;
    return ret;
}

void SharedRingBuffer::Packet::grantOwnershipOfData(ServerOp opcode, int sourceId, uint32_t len, byte* data)
{
    m_opcode    = opcode;
    m_sourceId  = sourceId;
    m_length    = len;
    m_data      = data;
}
