
#ifndef _EQP_CRC_HPP_
#define _EQP_CRC_HPP_

#include "define.hpp"
#include "netcode.hpp"

class CRC
{
private:
    static uint32_t update(const byte* data, uint32_t len, uint32_t crc = 0xffffffff);
    
public:
    static uint16_t calc16(const void* data, uint32_t len, uint32_t key);
    static uint16_t calc16NetworkByteOrder(const void* data, uint32_t len, uint32_t key);
    static uint32_t calc32(const void* data, uint32_t len);
    static uint32_t calc32NetworkByteOrder(const void* data, uint32_t len);
    static bool     validatePacket(const void* data, uint32_t len, uint32_t key);
};

#endif//_EQP_CRC_HPP_
