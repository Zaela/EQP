
#ifndef _EQP_ALIGNED_HPP_
#define _EQP_ALIGNED_HPP_

#include "define.hpp"
#include "exception.hpp"
#include <string>

namespace
{
    class AlignedIO
    {
    protected:
        uint32_t    m_cursor;
        uint32_t    m_length;
        byte*       m_buffer;

    protected:
        AlignedIO(void* ptr, uint32_t length) : m_cursor(0), m_length(length), m_buffer((byte*)ptr) { }
        
    public:
        inline uint32_t advance(uint32_t len)
        {
            uint32_t v = m_cursor;
            m_cursor += len;
            
            if (m_cursor > m_length)
                throw Exception("[AlignedWriter::advance] Attempt to write beyond end of buffer");
            
            return v;
        }
    };
}

class AlignedReader : public AlignedIO
{
public:
    AlignedReader(void* ptr, uint32_t length);

    ::byte byte() { return uint8(); }
    int8_t int8() { return (int8_t)uint8(); }
    uint8_t uint8();
    int16_t int16() { return (int16_t)uint16(); }
    uint16_t uint16();
    int int32() { return (int)uint32(); }
    uint32_t uint32();
    int64_t int64() { return (int64_t)uint64(); }
    uint64_t uint64();
};

class AlignedWriter : public AlignedIO
{
public:
    AlignedWriter(void* ptr, uint32_t length);

    inline void zeroAll() { memset(m_buffer, 0, m_length); }

    void byte(::byte value) { uint8(value); }
    void int8(int8_t value) { uint8((uint8_t)value); }
    void uint8(uint8_t value);
    void int16(int16_t value) { uint16((uint16_t)value); }
    void uint16(uint16_t value);
    void int32(int value) { uint32((uint32_t)value); }
    void uint32(uint32_t value);
    void int64(int64_t value) { uint64((uint64_t)value); }
    void uint64(uint64_t value);
    
    void string(const char* str, uint32_t length) { buffer((::byte*)str, length); }
    void string(const std::string& str) { string(str.c_str(), str.length()); }
    void stringNullTerminated(const std::string& str) { string(str.c_str(), str.length() + 1); }
    
    void buffer(::byte* data, uint32_t length);
};

#endif//_EQP_ALIGNED_HPP_
