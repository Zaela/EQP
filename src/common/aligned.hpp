
#ifndef _EQP_ALIGNED_HPP_
#define _EQP_ALIGNED_HPP_

#include "define.hpp"
#include "exception.hpp"
#include <string>
#include <sqlite3.h>

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
                throw Exception("[AlignedWriter::advance] Attempt to advance beyond end of buffer");
            
            return v;
        }
        
        inline void check(uint32_t len)
        {
            uint32_t c = m_cursor + len;
            if (c > m_length)
                throw Exception("[AlignedWriter::check] Attempt to check beyond end of buffer");
        }
        
        inline uint32_t size() const { return m_length; }
        inline uint32_t remaining() const { return m_length - m_cursor; }
        inline uint32_t position() const { return m_cursor; }
        inline byte* all() const { return m_buffer; }
        inline byte* current() const { return m_buffer + m_cursor; }
        
        void reduceSize(uint32_t by) { m_length -= by; }
        
        void reset()
        {
            m_cursor = 0;
        }
        
        void reset(uint32_t offset)
        {
            m_cursor = offset;
        }
        
        void reset(void* ptr, uint32_t length)
        {
            m_cursor = 0;
            m_length = length;
            m_buffer = (byte*)ptr;
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
    
    ::byte peekByte() { return peekUint8(); }
    int8_t peekInt8() { return (int8_t)peekUint8(); }
    uint8_t peekUint8();
    
    void buffer(void* dst, uint32_t len);
    
    // Returns the number of characters advanced over, not counting the null terminator
    // If the end of the buffer is reached without finding a null terminator, returns -1
    int advancePastNextNullTerminator();
};

class AlignedWriter : public AlignedIO
{
public:
    AlignedWriter(void* ptr, uint32_t length);

    inline void zeroAll() { memset(m_buffer, 0, m_length); }

    void boolean(bool value) { uint8((uint8_t)value); }
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
    void random(uint32_t bytes);
};

#endif//_EQP_ALIGNED_HPP_
