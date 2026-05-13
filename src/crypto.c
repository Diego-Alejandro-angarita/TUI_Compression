#include "crypto.h"

CryptoResult encrypt_buffer(const Buffer *plain, Buffer *encrypted, const CryptoContext *context)
{
    if (plain == NULL || encrypted == NULL || context == NULL) {
        return CRYPTO_ERR_INPUT;
    }

    (void)plain;
    (void)encrypted;
    (void)context;

    /* TODO: Cifrar el buffer comprimido en RAM con OpenSSL o cifrado simetrico propio. */
    return CRYPTO_ERR_NOT_IMPLEMENTED;
}

CryptoResult decrypt_buffer(const Buffer *encrypted, Buffer *plain, const CryptoContext *context)
{
    if (encrypted == NULL || plain == NULL || context == NULL) {
        return CRYPTO_ERR_INPUT;
    }

    (void)encrypted;
    (void)plain;
    (void)context;

    /* TODO: Descifrar en RAM antes de invocar la descompresion. */
    return CRYPTO_ERR_NOT_IMPLEMENTED;
}
