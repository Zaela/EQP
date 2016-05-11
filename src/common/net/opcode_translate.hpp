
#ifndef _EQP_OPCODE_TRANSLATE_HPP_
#define _EQP_OPCODE_TRANSLATE_HPP_

#include "define.hpp"

struct OpCodeTranslation
{
    uint16_t from;
    uint16_t to;
};

/*
uint16_t translateOpcodeFromServer(EQNet*, uint16_t opcode);
uint16_t translateOpcodeToServer(EQNet*, uint16_t opcode);
void initNoDeleteOpcodes();
void setNoDeleteOpcode(uint16_t opcode);
uint32_t isNoDeleteOpcode(uint16_t opcode);
void checkSpecialDestructor(EQNet_Packet& packet, int count);
*/

#endif//_EQP_OPCODE_TRANSLATE_HPP_
