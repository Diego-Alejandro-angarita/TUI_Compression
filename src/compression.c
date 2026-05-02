#include "compression.h"
#include "file_io.h"
#include <zstd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

CompressionResult compression_compress_file(
    const char *input_path,
    const char *output_path,
    StatsReport *stats
)
{
    char *input = NULL;
    size_t input_size = 0;

    if (file_io_read_all(input_path, &input, &input_size) != FILE_IO_OK)
        return COMPRESSION_ERR_ZSTD;

    size_t max_size = ZSTD_compressBound(input_size);
    void *compressed = malloc(max_size);
    if (!compressed) {
        free(input);
        return COMPRESSION_ERR_ZSTD;
    }

    clock_t start = clock();

    size_t compressed_size = ZSTD_compress(
        compressed,
        max_size,
        input,
        input_size,
        1
    );

    clock_t end = clock();

    if (ZSTD_isError(compressed_size)) {
        free(input);
        free(compressed);
        return COMPRESSION_ERR_ZSTD;
    }

    if (file_io_write_all(output_path, compressed, compressed_size) != FILE_IO_OK) {
        free(input);
        free(compressed);
        return COMPRESSION_ERR_ZSTD;
    }

    if (stats) {
        stats->bytes_written_classic  = input_size;
        stats->bytes_written_proposed = compressed_size;
        stats->wall_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        stats->user_time_ms = stats->wall_time_ms;
        stats->system_time_ms = 0.0;
    }

    free(input);
    free(compressed);
    return COMPRESSION_OK;
}

CompressionResult compression_decompress_file(
    const char *input_path,
    const char *output_path,
    StatsReport *stats
)
{
    char *input = NULL;
    size_t input_size = 0;

    if (file_io_read_all(input_path, &input, &input_size) != FILE_IO_OK)
        return COMPRESSION_ERR_ZSTD;

    unsigned long long original_size =
        ZSTD_getFrameContentSize(input, input_size);

    if (original_size == ZSTD_CONTENTSIZE_ERROR ||
        original_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        free(input);
        return COMPRESSION_ERR_ZSTD;
    }

    void *output = malloc((size_t)original_size);
    if (!output) {
        free(input);
        return COMPRESSION_ERR_ZSTD;
    }

    clock_t start = clock();

    size_t result = ZSTD_decompress(
        output,
        original_size,
        input,
        input_size
    );

    clock_t end = clock();

    if (ZSTD_isError(result)) {
        free(input);
        free(output);
        return COMPRESSION_ERR_ZSTD;
    }

    if (file_io_write_all(output_path, output, result) != FILE_IO_OK) {
        free(input);
        free(output);
        return COMPRESSION_ERR_ZSTD;
    }

    if (stats) {
        stats->bytes_written_classic  = input_size;
        stats->bytes_written_proposed = result;
        stats->wall_time_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    }

    free(input);
    free(output);
    return COMPRESSION_OK;
}