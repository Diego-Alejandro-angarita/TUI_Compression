#include "crypto.h"

int main(void)
{
    Buffer plain;
    Buffer encrypted;
    CryptoContext context = {0};

    buffer_init(&plain);
    buffer_init(&encrypted);
    context.algorithm = CRYPTO_AES_256_GCM;

    if (encrypt_buffer(&plain, &encrypted, &context) != CRYPTO_ERR_NOT_IMPLEMENTED) {
        return 1;
    }

    buffer_free(&encrypted);
    buffer_free(&plain);
    return 0;
}
