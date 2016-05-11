
void CharSelectClient::handleLoginInfoCanonical(AlignedReader& r)
{
    // info -> accountId as a string, null terminator, session key
    if (r.remaining() < sizeof(CanonicalStruct::LoginInfo))
        goto failure;
    
    // Scope for goto
    {
        const char* acctId  = (const char*)r.current();
        int acctLength      = r.advancePastNextNullTerminator();
        
        if (acctLength == -1 || acctLength > 64)
            goto failure;
        
        const char* sessionId   = (const char*)r.current();
        int sessionLength       = r.advancePastNextNullTerminator();
        
        if (sessionLength == -1 || (sessionLength + acctLength) >= 64 || sessionLength < 10)
            goto failure;
        
        setAccountId((uint32_t)std::stoi(acctId));
        printf("account %s, session %s\n", acctId, sessionId);
        
        //fixme: should not disconnect right away if this fails -- race condition with auth from login server
        if (socket().isClientAuthorized(ipAddress(), accountId(), sessionId))
            return;
    }
    
failure:
    disconnect();
}
