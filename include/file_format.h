#ifndef FILE_FORMAT_H
#define FILE_FORMAT_H

#include "compression.h"
#include "crypto.h"

#include <stddef.h>
#include <stdint.h>

#define FILE_FORMAT_MAGIC_SIZE 4u
#define FILE_FORMAT_SALT_SIZE 16u
#define FILE_FORMAT_NONCE_SIZE 24u

typedef enum {
    FILE_FORMAT_OK = 0,
    FILE_FORMAT_ERR_INPUT = -1,
    FILE_FORMAT_ERR_INVALID = -2,
    FILE_FORMAT_ERR_NOT_IMPLEMENTED = -3
} FileFormatResult;

typedef struct {
    uint8_t magic[FILE_FORMAT_MAGIC_SIZE];
    uint16_t version;
    CompressionAlgorithm compression_algorithm;
    CryptoAlgorithm crypto_algorithm;
    uint64_t original_size;
    uint64_t compressed_size;
    uint64_t encrypted_size;
    uint8_t salt[FILE_FORMAT_SALT_SIZE];
    uint8_t nonce[FILE_FORMAT_NONCE_SIZE];
} SecureFileHeader;

FileFormatResult file_format_create_header(
    SecureFileHeader *header,
    CompressionAlgorithm compression_algorithm,
    CryptoAlgorithm crypto_algorithm
);

FileFormatResult file_format_validate_header(const SecureFileHeader *header);

#endif /* FILE_FORMAT_H */
