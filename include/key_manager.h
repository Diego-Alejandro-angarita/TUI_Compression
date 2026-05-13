#ifndef KEY_MANAGER_H
#define KEY_MANAGER_H

#include <stddef.h>
#include <stdint.h>

#define KEY_MANAGER_MAX_KEY_SIZE 32u

typedef enum {
    KEY_MANAGER_OK = 0,
    KEY_MANAGER_ERR_INPUT = -1,
    KEY_MANAGER_ERR_NOT_IMPLEMENTED = -2
} KeyManagerResult;

typedef struct {
    uint8_t bytes[KEY_MANAGER_MAX_KEY_SIZE];
    size_t length;
    int is_initialized;
} SymmetricKey;

void key_manager_init_key(SymmetricKey *key);
KeyManagerResult key_manager_request_password_stub(char *password, size_t password_size);
KeyManagerResult key_manager_derive_key_stub(
    const char *password,
    const uint8_t *salt,
    size_t salt_len,
    SymmetricKey *key
);
void key_manager_destroy_key(SymmetricKey *key);

#endif /* KEY_MANAGER_H */
