
#ifndef _EQP_RING_BUFFER_HPP_
#define _EQP_RING_BUFFER_HPP_

#include "define.hpp"
#include "server_op.hpp"
#include <atomic>

// Single-reader, single-writer ring buffer, intended for shared memory regions
// Uses variable-size packets
class SharedRingBuffer
{
private:
    uint32_t                m_maxOffset;
    std::atomic<uint32_t>   m_startOffset;      // Modified by the consumer ONLY
    std::atomic<uint32_t>   m_endOffset;        // Modified by the producer ONLY
    std::atomic<uint32_t>   m_retreatOffset;    // Stores the filled endpoint (where to read up to) when the endOffset restarts from 0
    byte                    m_memoryRegion[4];  // Variable-length stub

private:
    struct InternalPacket
    {
    public:
        ServerOp    m_opcode;
        int         m_sourceId;
        uint32_t    m_length;
        uint32_t    m_totalLength;
        byte        m_data[4];  // Variable-length stub
        
    public:
        ServerOp opcode() const { return m_opcode; }
        int sourceId() const { return m_sourceId; }
        uint32_t length() const { return m_length; }
        uint32_t totalLength() const { return m_totalLength; }
        const byte* data() const { return m_data; }
    };
    
    static const uint32_t PACKET_OVERHEAD = offsetof(InternalPacket, m_data);
    
public:
    class Packet
    {
    private:
        ServerOp    m_opcode;
        int         m_sourceId;
        uint32_t    m_length;
        byte*       m_data;

    private:
        friend class SharedRingBuffer;
        void set(ServerOp opcode, int sourceId, uint32_t len, const byte* data);
    
    public:
        Packet();
        Packet(ServerOp opcode, int sourceId, uint32_t len, const byte* data);
        Packet(Packet&& o);
        ~Packet();
    
        Packet& operator=(Packet&& o);
    
        ServerOp opcode() const { return m_opcode; }
        int sourceId() const { return m_sourceId; }
        uint32_t length() const { return m_length; }
        byte* data() { return m_data; }
        
        byte* takeOwnershipOfData();
        void grantOwnershipOfData(ServerOp opcode, int sourceId, uint32_t len, byte* data);
    };

private:
    static uint32_t calcPacketLength(uint32_t len);

public:
    void init(uint32_t totalSize);

    bool pop(Packet& out);
    bool push(ServerOp opcode, int sourceId, uint32_t len, const byte* data);
};

typedef SharedRingBuffer::Packet IpcPacket;

#endif//_EQP_RING_BUFFER_HPP_
