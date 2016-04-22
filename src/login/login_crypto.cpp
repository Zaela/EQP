
#include "login_crypto.hpp"

LoginCrypto::LoginCrypto()
{
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(nullptr);
    
    // Init key schedule
    DES_cblock key = {0, 0, 0, 0, 0, 0, 0, 0};
    DES_key_sched(&key, &m_keySchedule);
}

LoginCrypto::~LoginCrypto()
{
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
}

uint32_t LoginCrypto::crypt(const void* input, uint32_t length, int encrypt)
{
    uint32_t rem = length % 8;
    
    if (rem)
        length += 8 - rem;
    
    if (length > BUFFER_SIZE)
        length = BUFFER_SIZE;
    
    DES_cblock iv = {0, 0, 0, 0, 0, 0, 0, 0};
    DES_ncbc_encrypt((const byte*)input, m_buffer, length, &m_keySchedule, &iv, encrypt);
    
    return length;
}

uint32_t LoginCrypto::encrypt(const void* input, uint32_t length)
{
    return crypt(input, length, 1);
}

uint32_t LoginCrypto::decrypt(const void* input, uint32_t length)
{
    return crypt(input, length, 0);
}

void LoginCrypto::hash(const char* pass, uint32_t passlen, const byte* salt, uint32_t saltlen)
{
    PKCS5_PBKDF2_HMAC_SHA1(pass, passlen, salt, saltlen, HASH_ITERATIONS, HASH_SIZE, m_buffer); 
}
