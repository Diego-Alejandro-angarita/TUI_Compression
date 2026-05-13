#include "io_pipeline.h"

#include "file_format.h"
#include "key_manager.h"

#include <stddef.h>

IoPipelineResult io_pipeline_init_default(IoPipelineConfig *config)
{
    if (config == NULL) {
        return IO_PIPELINE_ERR_INPUT;
    }

    config->compression_algorithm = COMPRESSION_ZSTD;
    config->crypto_algorithm = CRYPTO_AES_256_GCM;
    return IO_PIPELINE_OK;
}

IoPipelineResult io_pipeline_save_stub(
    const Buffer *plain_text,
    const char *path,
    const IoPipelineConfig *config,
    ProfileReport *profile)
{
    Buffer compressed;
    Buffer encrypted;
    SecureFileHeader header;
    SymmetricKey key;
    CryptoContext crypto_context;

    if (plain_text == NULL || path == NULL || config == NULL) {
        return IO_PIPELINE_ERR_INPUT;
    }

    (void)path;
    profiler_report_init(profile);
    buffer_init(&compressed);
    buffer_init(&encrypted);
    key_manager_init_key(&key);

    if (file_format_create_header(
            &header,
            config->compression_algorithm,
            config->crypto_algorithm) != FILE_FORMAT_OK) {
        key_manager_destroy_key(&key);
        return IO_PIPELINE_ERR_FORMAT;
    }

    /* TODO: Solicitar password y derivar llave real con KDF antes de cifrar. */
    crypto_context.algorithm = config->crypto_algorithm;
    crypto_context.key = key.bytes;
    crypto_context.key_len = key.length;
    crypto_context.nonce = header.nonce;
    crypto_context.nonce_len = sizeof(header.nonce);

    /* Flujo futuro: texto plano -> compresion -> cifrado -> escritura. */
    (void)compress_buffer(plain_text, &compressed, config->compression_algorithm);
    (void)encrypt_buffer(&compressed, &encrypted, &crypto_context);

    /* TODO: Escribir header + encrypted usando buffers y write(). */

    buffer_free(&encrypted);
    buffer_free(&compressed);
    key_manager_destroy_key(&key);

    return IO_PIPELINE_ERR_NOT_IMPLEMENTED;
}

IoPipelineResult io_pipeline_load_stub(
    const char *path,
    Buffer *plain_text,
    const IoPipelineConfig *config,
    ProfileReport *profile)
{
    Buffer encrypted;
    Buffer compressed;
    SymmetricKey key;
    CryptoContext crypto_context;

    if (path == NULL || plain_text == NULL || config == NULL) {
        return IO_PIPELINE_ERR_INPUT;
    }

    (void)path;
    profiler_report_init(profile);
    buffer_init(&encrypted);
    buffer_init(&compressed);
    key_manager_init_key(&key);

    crypto_context.algorithm = config->crypto_algorithm;
    crypto_context.key = key.bytes;
    crypto_context.key_len = key.length;
    crypto_context.nonce = NULL;
    crypto_context.nonce_len = 0;

    /* Flujo futuro: lectura -> descifrado -> descompresion -> texto plano. */
    (void)decrypt_buffer(&encrypted, &compressed, &crypto_context);
    (void)decompress_buffer(&compressed, plain_text, config->compression_algorithm);

    buffer_free(&compressed);
    buffer_free(&encrypted);
    key_manager_destroy_key(&key);

    return IO_PIPELINE_ERR_NOT_IMPLEMENTED;
}

IoPipelineResult io_pipeline_demo(void)
{
    IoPipelineConfig config;

    if (io_pipeline_init_default(&config) != IO_PIPELINE_OK) {
        return IO_PIPELINE_ERR_INPUT;
    }

    /* Demo estructural: no ejecuta compresion, cifrado, I/O ni TUI reales. */
    return IO_PIPELINE_OK;
}
