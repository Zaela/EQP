
#ifndef _EQP_LOGIN_CRYPTO_HPP_
#define _EQP_LOGIN_CRYPTO_HPP_

#include "define.hpp"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/des.h>

class LoginCrypto
{
private:
    static const int BUFFER_SIZE        = 2048;
    static const int HASH_SIZE          = 20;    // Recommended maximum for PBKDF2 using SHA1
    static const int HASH_ITERATIONS    = 50000; // Takes ~80 milliseconds on a 1.90GHz laptop CPU core

private:
    DES_key_schedule    m_keySchedule;
    DES_key_schedule    m_keyScheduleTrilogy;
    byte                m_buffer[BUFFER_SIZE];

private:
    uint32_t crypt(const void* input, uint32_t length, int encrypt, bool isTrilogy = false);
    
public:
    LoginCrypto();
    ~LoginCrypto();

    uint32_t encrypt(const void* input, uint32_t length, bool isTrilogy = false);
    uint32_t decrypt(const void* input, uint32_t length, bool isTrilogy = false);

    void hash(const char* pass, uint32_t passlen, const byte* salt, uint32_t saltlen);

    static int hashSize() { return HASH_SIZE; }

    const byte* data() { return m_buffer; }
    void clear() { memset(m_buffer, 0, BUFFER_SIZE); }
};

#endif//_EQP_LOGIN_CRYPTO_HPP_
