
#include "aligned.hpp"

/*================================================================================*\
** AlignedReader
\*================================================================================*/

AlignedReader::AlignedReader(void* ptr, uint32_t length)
: AlignedIO(ptr, length)
{
    
}

uint8_t AlignedReader::uint8()
{
    uint32_t c = advance(sizeof(uint8_t));
    return m_buffer[c];
}

uint16_t AlignedReader::uint16()
{
    uint32_t c = advance(sizeof(uint16_t));
    
    uint16_t ret;
    
    ret  = m_buffer[c + 0] << 0;
    ret |= m_buffer[c + 1] << 8;
    
    return ret;
}

uint32_t AlignedReader::uint32()
{
    uint32_t c = advance(sizeof(uint32_t));
    
    uint32_t ret;
    
    ret  = m_buffer[c + 0] <<  0;
    ret |= m_buffer[c + 1] <<  8;
    ret |= m_buffer[c + 2] << 16;
    ret |= m_buffer[c + 3] << 24;
    
    return ret;
}

uint64_t AlignedReader::uint64()
{
    uint32_t c = advance(sizeof(uint64_t));
    
    uint64_t ret;
    
    ret  = ((uint64_t)m_buffer[c + 0]) <<  0;
    ret |= ((uint64_t)m_buffer[c + 1]) <<  8;
    ret |= ((uint64_t)m_buffer[c + 2]) << 16;
    ret |= ((uint64_t)m_buffer[c + 3]) << 24;
    ret |= ((uint64_t)m_buffer[c + 4]) << 32;
    ret |= ((uint64_t)m_buffer[c + 5]) << 40;
    ret |= ((uint64_t)m_buffer[c + 6]) << 48;
    ret |= ((uint64_t)m_buffer[c + 7]) << 56;
    
    return ret;
}

void AlignedReader::buffer(void* ptr, uint32_t len)
{
    ::byte* dst = (::byte*)ptr;
    uint32_t c  = advance(len);
    
    for (uint32_t i = 0; i < len; i++)
    {
        dst[i] = m_buffer[c + i];
    }
}

/*================================================================================*\
** AlignedWriter
\*================================================================================*/

AlignedWriter::AlignedWriter(void* ptr, uint32_t length)
: AlignedIO(ptr, length)
{
    
}

void AlignedWriter::uint8(uint8_t value)
{
    uint32_t c = advance(sizeof(uint8_t));
    m_buffer[c] = value;
}

void AlignedWriter::uint16(uint16_t value)
{
    uint32_t c = advance(sizeof(uint16_t));
    
    m_buffer[c + 0] = (uint8_t)((value & 0x00ff) >> 0);
    m_buffer[c + 1] = (uint8_t)((value & 0xff00) >> 8);
}

void AlignedWriter::uint32(uint32_t value)
{
    uint32_t c = advance(sizeof(uint32_t));
    
    m_buffer[c + 0] = (uint8_t)((value & 0x000000ff) >>  0);
    m_buffer[c + 1] = (uint8_t)((value & 0x0000ff00) >>  8);
    m_buffer[c + 2] = (uint8_t)((value & 0x00ff0000) >> 16);
    m_buffer[c + 3] = (uint8_t)((value & 0xff000000) >> 24);
}

void AlignedWriter::uint64(uint64_t value)
{
    uint32_t c = advance(sizeof(uint64_t));
    
    m_buffer[c + 0] = (uint8_t)((value & 0x00000000000000ff) >>  0);
    m_buffer[c + 1] = (uint8_t)((value & 0x000000000000ff00) >>  8);
    m_buffer[c + 2] = (uint8_t)((value & 0x0000000000ff0000) >> 16);
    m_buffer[c + 3] = (uint8_t)((value & 0x00000000ff000000) >> 24);
    m_buffer[c + 4] = (uint8_t)((value & 0x000000ff00000000) >> 32);
    m_buffer[c + 5] = (uint8_t)((value & 0x0000ff0000000000) >> 40);
    m_buffer[c + 6] = (uint8_t)((value & 0x00ff000000000000) >> 48);
    m_buffer[c + 7] = (uint8_t)((value & 0xff00000000000000) >> 56);
}

void AlignedWriter::buffer(::byte* data, uint32_t len)
{
    uint32_t c = advance(len);
    
    for (uint32_t i = 0; i < len; i++)
    {
        m_buffer[c + i] = data[i];
    }
}

void AlignedWriter::random(uint32_t bytes)
{
    uint32_t c = advance(bytes);
    
    sqlite3_randomness((int)bytes, m_buffer + c);
}
