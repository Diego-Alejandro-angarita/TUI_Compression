#include "file_format.h"

#include <string.h>

static const uint8_t secure_file_magic[FILE_FORMAT_MAGIC_SIZE] = {'S', 'C', 'Z', '1'};

FileFormatResult file_format_create_header(
    SecureFileHeader *header,
    CompressionAlgorithm compression_algorithm,
    CryptoAlgorithm crypto_algorithm)
{
    if (header == NULL) {
        return FILE_FORMAT_ERR_INPUT;
    }

    memset(header, 0, sizeof(*header));
    memcpy(header->magic, secure_file_magic, sizeof(header->magic));
    header->version = 1;
    header->compression_algorithm = compression_algorithm;
    header->crypto_algorithm = crypto_algorithm;

    /* TODO: Rellenar salt y nonce con bytes aleatorios criptograficos. */
    return FILE_FORMAT_OK;
}

FileFormatResult file_format_validate_header(const SecureFileHeader *header)
{
    if (header == NULL) {
        return FILE_FORMAT_ERR_INPUT;
    }

    if (memcmp(header->magic, secure_file_magic, FILE_FORMAT_MAGIC_SIZE) != 0) {
        return FILE_FORMAT_ERR_INVALID;
    }

    if (header->version != 1) {
        return FILE_FORMAT_ERR_INVALID;
    }

    /* TODO: Validar algoritmos soportados y consistencia de tamanos. */
    return FILE_FORMAT_OK;
}
