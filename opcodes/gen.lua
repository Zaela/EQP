
local tonumber  = tonumber
local string    = string
local table     = table

local translations = {
    OP_MOTD             = "OP_MessageOfTheDay",
    OP_ZoneUnavail      = "OP_ZoneUnavailable",
    OP_TimeOfDay        = "OP_TimeUpdate",
    OP_NewSpawn         = "OP_Spawn",
    OP_DeleteSpawn      = "OP_Despawn",
    OP_ZoneEntry        = "OP_PlayerSpawn",
    OP_Stamina          = "OP_HungerThirstUpdate",
    OP_ManaChange       = "OP_ManaEnduranceUpdate",
    OP_MobUpdate        = "OP_PositionUpdate",
    OP_HPUpdate         = "OP_HpUpdateExact",
    OP_MobHealth        = "OP_HpUpdatePercent",
    OP_NewZone          = "OP_ZoneData",
    OP_SendExpZonein    = "OP_SetExperience",
    OP_Weather          = "OP_WeatherUpdate",
    OP_SpawnDoor        = "OP_DoorSpawn",
    OP_ExpUpdate        = "OP_ExperienceUpdate",
    OP_TargetHoTT       = "OP_TargetsTarget",
    OP_ChannelMessage   = "OP_ChatMessage",
    OP_FormattedMessage = "OP_ChatMessageEQStr",
}

local opcodeNames = {}
local opcodeNamesOrdered = {}

local function gen(client, namespace)
    local file  = assert(io.open(client .. ".txt", "r"))
    local str   = file:read("*a")
    file:close()

    local out   = io.open("../src/common/net/opcodes_".. client ..".hpp", "w+")
    local caps  = client:upper()
    
    local found     = {}
    local opcodes   = {}
    
    -- Translate opcodes
	for opname, val in str:gmatch("\n%s*OP_(%w+)%s*=%s*(0x%w+)") do
        if val == "0x0000" then goto skip end
        if tonumber(val) <= 0x29 then goto skip end -- Login opcodes

        local tname = translations[opname] or opname
        val = val:lower()

        if #val < 6 then
            val = "0x" .. string.rep("0", 6 - #val) .. val:sub(3)
        end

        if not opcodeNames[tname] then
            opcodeNames[tname] = true
            table.insert(opcodeNamesOrdered, tname)
        end
        
        if not found[tname] then
            found[tname] = true
            table.insert(opcodes, tname)
            table.insert(opcodes, val)
		end
        
        ::skip::
    end
    
    -- Write header
    out:write(string.format([[

#ifndef _EQP_OPCODES_%s_HPP_
#define _EQP_OPCODES_%s_HPP_

#include "define.hpp"
#include "opcodes_canonical.hpp"
#include "opcode_translate.hpp"

namespace %s
{
    enum Op : uint16_t
    {
]], caps, caps, namespace))

    -- Write enum values
    for i = 1, #opcodes, 2 do
        local name, value = opcodes[i], opcodes[i + 1]
        
        out:write(string.format("        %s = %s,\n", name, value))
    end
    
    out:write([[
        COUNT
    };

    static const OpCodeTranslation toCanonical[] = {
]])

    -- Write translation pairs
    for i = 1, #opcodes, 2 do
        local name, value = opcodes[i], opcodes[i + 1]
        
        if i ~= 1 then
            out:write(string.format(",\n"))
		end
        
        first = false
        out:write(string.format("        { %s, CanonicalOp::%s }", value, name))
    end
    
    -- Write footer
    out:write(string.format([[

    };
}

#endif//_EQP_OPCODES_%s_HPP_
]], caps))

    out:close()
end

gen("titanium", "Titanium")
gen("sof", "SecretsOfFaydwer")
gen("sod", "SeedsOfDestruction")
gen("underfoot", "Underfoot")
gen("rof", "ReignOfFear")
gen("rof2", "ReignOfFear2")

local file  = io.open("../src/common/net/opcodes_canonical.hpp", "r")
local str   = file:read("*a")
file:close()

local enum = {"enum CanonicalOp : uint16_t\n{\n    NONE,\n"}
for i, opname in ipairs(opcodeNamesOrdered) do
    table.insert(enum, "    ".. opname ..",\n")
end
table.insert(enum, "    COUNT\n}")

str = str:gsub("enum CanonicalOp : uint16_t[\r\n]+{[\r\n]+.-}", table.concat(enum))

local out = io.open("../src/common/net/opcodes_canonical.hpp", "w+")
out:write(str)
out:close()
