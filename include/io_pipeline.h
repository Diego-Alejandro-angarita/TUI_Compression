#ifndef IO_PIPELINE_H
#define IO_PIPELINE_H

#include "buffer.h"
#include "compression.h"
#include "crypto.h"
#include "profiler.h"

typedef enum {
    IO_PIPELINE_OK = 0,
    IO_PIPELINE_ERR_INPUT = -1,
    IO_PIPELINE_ERR_NOT_IMPLEMENTED = -2,
    IO_PIPELINE_ERR_COMPRESSION = -3,
    IO_PIPELINE_ERR_CRYPTO = -4,
    IO_PIPELINE_ERR_FORMAT = -5
} IoPipelineResult;

typedef struct {
    CompressionAlgorithm compression_algorithm;
    CryptoAlgorithm crypto_algorithm;
} IoPipelineConfig;

IoPipelineResult io_pipeline_init_default(IoPipelineConfig *config);
IoPipelineResult io_pipeline_save_stub(
    const Buffer *plain_text,
    const char *path,
    const IoPipelineConfig *config,
    ProfileReport *profile
);
IoPipelineResult io_pipeline_load_stub(
    const char *path,
    Buffer *plain_text,
    const IoPipelineConfig *config,
    ProfileReport *profile
);
IoPipelineResult io_pipeline_demo(void);

#endif /* IO_PIPELINE_H */
