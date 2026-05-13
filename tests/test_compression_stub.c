#include "compression.h"

int main(void)
{
    Buffer plain;
    Buffer compressed;

    buffer_init(&plain);
    buffer_init(&compressed);

    if (compress_buffer(&plain, &compressed, COMPRESSION_ZSTD) != COMPRESSION_ERR_NOT_IMPLEMENTED) {
        return 1;
    }

    buffer_free(&compressed);
    buffer_free(&plain);
    return 0;
}
