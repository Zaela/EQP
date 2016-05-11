
#ifndef EQP_DISABLE_TITANIUM

void handlePacketTitanium(AlignedReader& r, uint16_t opcode)
{
    switch (opcode)
    {
    case Titanium::Op::SendLoginInfo:
        handleLoginInfoCanonical(r);
        break;
    
    default:
        break;
    }
}

#endif//EQP_DISABLE_TITANIUM
