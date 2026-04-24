#include "compression.h"

CompressionResult compression_compress_file(
    const char *input_path,
    const char *output_path,
    StatsReport *stats
)
{
    (void)input_path;
    (void)output_path;
    (void)stats;

    return COMPRESSION_ERR_ZSTD;
}

CompressionResult compression_decompress_file(
    const char *input_path,
    const char *output_path,
    StatsReport *stats
)
{
    (void)input_path;
    (void)output_path;
    (void)stats;

    return COMPRESSION_ERR_ZSTD;
}

