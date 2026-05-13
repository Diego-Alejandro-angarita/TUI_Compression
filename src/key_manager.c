#include "key_manager.h"

#include "secure_memory.h"

#include <string.h>

void key_manager_init_key(SymmetricKey *key)
{
    if (key == NULL) {
        return;
    }

    memset(key, 0, sizeof(*key));
}

KeyManagerResult key_manager_request_password_stub(char *password, size_t password_size)
{
    if (password == NULL || password_size == 0) {
        return KEY_MANAGER_ERR_INPUT;
    }

    password[0] = '\0';

    /* TODO: Solicitar password sin eco desde la TUI o entrada segura. */
    return KEY_MANAGER_ERR_NOT_IMPLEMENTED;
}

KeyManagerResult key_manager_derive_key_stub(
    const char *password,
    const uint8_t *salt,
    size_t salt_len,
    SymmetricKey *key)
{
    if (password == NULL || salt == NULL || salt_len == 0 || key == NULL) {
        return KEY_MANAGER_ERR_INPUT;
    }

    key_manager_init_key(key);

    /* TODO: Implementar KDF real, por ejemplo PBKDF2, scrypt o Argon2. */
    return KEY_MANAGER_ERR_NOT_IMPLEMENTED;
}

void key_manager_destroy_key(SymmetricKey *key)
{
    if (key == NULL) {
        return;
    }

    secure_zero(key, sizeof(*key));
}
