#include "compression.h"

CompressionResult compress_buffer(
    const Buffer *plain,
    Buffer *compressed,
    CompressionAlgorithm algorithm)
{
    if (plain == NULL || compressed == NULL) {
        return COMPRESSION_ERR_INPUT;
    }

    (void)plain;
    (void)compressed;
    (void)algorithm;

    /* TODO: Integrar ZSTD aqui usando buffers en RAM, no archivos directos. */
    return COMPRESSION_ERR_NOT_IMPLEMENTED;
}

CompressionResult decompress_buffer(
    const Buffer *compressed,
    Buffer *plain,
    CompressionAlgorithm algorithm)
{
    if (compressed == NULL || plain == NULL) {
        return COMPRESSION_ERR_INPUT;
    }

    (void)compressed;
    (void)plain;
    (void)algorithm;

    /* TODO: Integrar ZSTD_decompress para recuperar texto plano en memoria. */
    return COMPRESSION_ERR_NOT_IMPLEMENTED;
}
