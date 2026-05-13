#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "buffer.h"

typedef enum {
    COMPRESSION_OK = 0,
    COMPRESSION_ERR_INPUT = -1,
    COMPRESSION_ERR_NOT_IMPLEMENTED = -2
} CompressionResult;

typedef enum {
    COMPRESSION_NONE = 0,
    COMPRESSION_ZSTD = 1
} CompressionAlgorithm;

CompressionResult compress_buffer(
    const Buffer *plain,
    Buffer *compressed,
    CompressionAlgorithm algorithm
);

CompressionResult decompress_buffer(
    const Buffer *compressed,
    Buffer *plain,
    CompressionAlgorithm algorithm
);

#endif /* COMPRESSION_H */
