#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "stats.h"

typedef enum {
    COMPRESSION_OK = 0,
    COMPRESSION_ERR_INPUT,
    COMPRESSION_ERR_OUTPUT,
    COMPRESSION_ERR_ZSTD
} CompressionResult;

CompressionResult compression_compress_file(
    const char  *input_path,
    const char  *output_path,
    StatsReport *stats
);

CompressionResult compression_decompress_file(
    const char  *input_path,
    const char  *output_path,
    StatsReport *stats
);

#endif /* COMPRESSION_H */