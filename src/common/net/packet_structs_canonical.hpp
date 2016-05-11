
#ifndef _EQP_PACKET_STRUCTS_CANONICAL_HPP_
#define _EQP_PACKET_STRUCTS_CANONICAL_HPP_

namespace CanonicalStruct
{
#pragma pack(1)
    
    struct LoginInfo
    {
        char    info[64];
        uint8_t unknownA[124];
        uint8_t zoning;
        uint8_t unknownB[275];
    };
    
#pragma pack()
}; //namespace CanonicalStruct

#endif//_EQP_PACKET_STRUCTS_CANONICAL_HPP_
