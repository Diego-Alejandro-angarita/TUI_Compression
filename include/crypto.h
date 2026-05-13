#ifndef CRYPTO_H
#define CRYPTO_H

#include "buffer.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CRYPTO_OK = 0,
    CRYPTO_ERR_INPUT = -1,
    CRYPTO_ERR_NOT_IMPLEMENTED = -2
} CryptoResult;

typedef enum {
    CRYPTO_NONE = 0,
    CRYPTO_CHACHA20 = 1,
    CRYPTO_AES_256_GCM = 2
} CryptoAlgorithm;

typedef struct {
    CryptoAlgorithm algorithm;
    const uint8_t *key;
    size_t key_len;
    const uint8_t *nonce;
    size_t nonce_len;
} CryptoContext;

CryptoResult encrypt_buffer(const Buffer *plain, Buffer *encrypted, const CryptoContext *context);
CryptoResult decrypt_buffer(const Buffer *encrypted, Buffer *plain, const CryptoContext *context);

#endif /* CRYPTO_H */
