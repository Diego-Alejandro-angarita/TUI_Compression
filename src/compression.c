/*
 * src/compression.c  —  Persona 2: Compresión, descompresión y escritura.
 *
 * ─── Decisiones técnicas ────────────────────────────────────────────────────
 *
 * 1. FORMATO DE ARCHIVO COMPRIMIDO
 *    Cada línea del archivo fuente se comprime como un frame Zstd independiente.
 *    El archivo resultante es una secuencia de registros:
 *
 *        [ uint32_t frame_len ][ <frame_len bytes Zstd> ]
 *        [ uint32_t frame_len ][ <frame_len bytes Zstd> ]
 *        ...
 *
 *    Esto permite descomprimir línea a línea sin conocer el tamaño total.
 *
 * 2. ESCRITURA EN BLOQUES DE 4 KB
 *    Un WriteBuffer de OUTPUT_BUF (4096 bytes) acumula datos antes de llamar
 *    a write(). Reduce syscalls al mínimo necesario, lo que impacta directamente
 *    en las métricas de write_calls y tiempo en modo kernel (sys mode).
 *
 * 3. DECISIÓN SOBRE mmap
 *    Se evaluó mmap() para lectura y escritura.
 *    → Lectura: DESCARTADO. El procesamiento línea a línea requiere buscar '\n'
 *      byte a byte; fread() con buffer de 64 KB tiene el mismo costo de syscalls.
 *    → Escritura: DESCARTADO. El bloque de 4 KB con write() ya minimiza syscalls
 *      sin la complejidad extra de mmap + msync + munmap.
 *
 * 4. SIN DEPENDENCIA DE ncurses.
 *
 * ─── Dependencias ───────────────────────────────────────────────────────────
 *    libzstd-dev:  sudo apt install libzstd-dev
 * ────────────────────────────────────────────────────────────────────────────
 */

#define _POSIX_C_SOURCE 200809L

#include "compression.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <zstd.h>

/* ── Constantes ─────────────────────────────────────────────────────────── */

#define OUTPUT_BUF   4096u           /* tamaño del bloque de escritura       */
#define LINE_MAX_LEN (1u << 20)      /* longitud máxima de una línea: 1 MB   */
#define FRAME_HDR    sizeof(uint32_t)/* bytes del header de frame propio     */
#define ZSTD_LEVEL   3               /* nivel de compresión por defecto      */

/* ── Buffer de escritura con flush automático ───────────────────────────── */

typedef struct {
    int      fd;
    uint8_t  data[OUTPUT_BUF];
    size_t   used;
    uint64_t write_calls;
    uint64_t total_written;
} WBuf;

static void wbuf_init(WBuf *w, int fd)
{
    w->fd          = fd;
    w->used        = 0;
    w->write_calls = 0;
    w->total_written = 0;
}

static int wbuf_write_full(WBuf *w, const uint8_t *src, size_t len)
{
    size_t written = 0;

    while (written < len) {
        ssize_t n = write(w->fd, src + written, len - written);
        w->write_calls++;

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (n == 0) {
            return -1;
        }

        written += (size_t)n;
        w->total_written += (uint64_t)n;
    }

    return 0;
}

/* Devuelve 0 en éxito, -1 en error de write(). */
static int wbuf_push(WBuf *w, const uint8_t *src, size_t len)
{
    while (len > 0) {
        size_t space = OUTPUT_BUF - w->used;
        size_t copy  = len < space ? len : space;
        memcpy(w->data + w->used, src, copy);
        w->used += copy;
        src     += copy;
        len     -= copy;

        if (w->used == OUTPUT_BUF) {
            if (wbuf_write_full(w, w->data, OUTPUT_BUF) != 0) {
                return -1;
            }
            w->used = 0;
        }
    }
    return 0;
}

/* Vuelca el remanente del buffer a disco. */
static int wbuf_flush(WBuf *w)
{
    if (w->used == 0) return 0;
    if (wbuf_write_full(w, w->data, w->used) != 0) {
        return -1;
    }
    w->used = 0;
    return 0;
}

/* ── compression_compress_file ──────────────────────────────────────────── */

CompressionResult compression_compress_file(
    const char  *input_path,
    const char  *output_path,
    StatsReport *stats)
{
    if (!input_path || !output_path)
        return COMPRESSION_ERR_INPUT;

    StatsTimer timer;
    stats_timer_start(&timer);

    /* Abrir fuente */
    FILE *in = fopen(input_path, "rb");
    if (!in) return COMPRESSION_ERR_INPUT;

    /* Abrir destino con write() para usar el bloque de 4 KB */
    int out_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) { fclose(in); return COMPRESSION_ERR_OUTPUT; }

    /* Buffers */
    uint8_t *line_buf  = malloc(LINE_MAX_LEN);
    size_t   comp_bound = ZSTD_compressBound(LINE_MAX_LEN);
    uint8_t *comp_buf  = malloc(comp_bound);

    if (!line_buf || !comp_buf) {
        free(line_buf); free(comp_buf);
        fclose(in); close(out_fd);
        return COMPRESSION_ERR_ZSTD;
    }

    WBuf wb;
    wbuf_init(&wb, out_fd);

    uint64_t bytes_orig = 0;
    CompressionResult result = COMPRESSION_OK;

    /* ── Iterar línea a línea ── */
    while (!feof(in)) {
        size_t line_len = 0;
        int c;
        while (line_len < LINE_MAX_LEN - 1 && (c = fgetc(in)) != EOF) {
            line_buf[line_len++] = (uint8_t)c;
            if (c == '\n') break;
        }
        if (line_len == 0) break;

        bytes_orig += line_len;

        /* Comprimir la línea */
        size_t csize = ZSTD_compress(comp_buf, comp_bound,
                                     line_buf, line_len, ZSTD_LEVEL);
        if (ZSTD_isError(csize)) {
            result = COMPRESSION_ERR_ZSTD;
            goto done;
        }

        /* Escribir: [uint32_t tamaño][datos] */
        uint32_t frame_len = (uint32_t)csize;
        if (wbuf_push(&wb, (const uint8_t *)&frame_len, FRAME_HDR) != 0 ||
            wbuf_push(&wb, comp_buf, csize) != 0) {
            result = COMPRESSION_ERR_OUTPUT;
            goto done;
        }
    }

    if (wbuf_flush(&wb) != 0)
        result = COMPRESSION_ERR_OUTPUT;

done:
    free(line_buf);
    free(comp_buf);
    fclose(in);
    close(out_fd);

    if (stats && result == COMPRESSION_OK) {
        stats_add_original_bytes(stats, bytes_orig);
        stats_add_compressed_bytes(stats, wb.total_written);
        stats_add_written_bytes(stats, wb.total_written);
        stats_add_write_calls(stats, wb.write_calls);
        stats_timer_stop(&timer, stats);
    }

    return result;
}

/* ── compression_decompress_file ────────────────────────────────────────── */

CompressionResult compression_decompress_file(
    const char  *input_path,
    const char  *output_path,
    StatsReport *stats)
{
    if (!input_path || !output_path)
        return COMPRESSION_ERR_INPUT;

    StatsTimer timer;
    stats_timer_start(&timer);

    FILE *in = fopen(input_path, "rb");
    if (!in) return COMPRESSION_ERR_INPUT;

    int out_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) { fclose(in); return COMPRESSION_ERR_OUTPUT; }

    /* Capacidades iniciales; se agrandan con realloc si hace falta */
    size_t   frame_cap  = ZSTD_compressBound(LINE_MAX_LEN);
    uint8_t *frame_buf  = malloc(frame_cap);
    size_t   decomp_cap = LINE_MAX_LEN;
    uint8_t *decomp_buf = malloc(decomp_cap);

    if (!frame_buf || !decomp_buf) {
        free(frame_buf); free(decomp_buf);
        fclose(in); close(out_fd);
        return COMPRESSION_ERR_ZSTD;
    }

    WBuf wb;
    wbuf_init(&wb, out_fd);

    uint64_t bytes_orig = 0;
    CompressionResult result = COMPRESSION_OK;

    while (!feof(in)) {
        /* Leer header del frame */
        uint32_t frame_len = 0;
        size_t n = fread(&frame_len, 1, FRAME_HDR, in);
        if (n == 0) break;          /* EOF limpio */
        if (n < FRAME_HDR) { result = COMPRESSION_ERR_INPUT; goto done_d; }

        /* Crecer buffer de frame si hace falta */
        if (frame_len > frame_cap) {
            uint8_t *tmp = realloc(frame_buf, frame_len);
            if (!tmp) { result = COMPRESSION_ERR_ZSTD; goto done_d; }
            frame_buf = tmp;
            frame_cap = frame_len;
        }

        /* Leer datos del frame */
        n = fread(frame_buf, 1, frame_len, in);
        if (n < frame_len) { result = COMPRESSION_ERR_INPUT; goto done_d; }

        /* Averiguar tamaño descomprimido */
        unsigned long long content_sz =
            ZSTD_getFrameContentSize(frame_buf, frame_len);

        if (content_sz == ZSTD_CONTENTSIZE_ERROR) {
            result = COMPRESSION_ERR_ZSTD;
            goto done_d;
        }

        /* Crecer buffer de descompresión */
        if (content_sz == ZSTD_CONTENTSIZE_UNKNOWN ||
            (size_t)content_sz > decomp_cap) {
            size_t new_cap = (content_sz == ZSTD_CONTENTSIZE_UNKNOWN)
                             ? decomp_cap * 2
                             : (size_t)content_sz;
            uint8_t *tmp = realloc(decomp_buf, new_cap);
            if (!tmp) { result = COMPRESSION_ERR_ZSTD; goto done_d; }
            decomp_buf = tmp;
            decomp_cap = new_cap;
        }

        size_t dsize = ZSTD_decompress(decomp_buf, decomp_cap,
                                       frame_buf, frame_len);
        if (ZSTD_isError(dsize)) {
            result = COMPRESSION_ERR_ZSTD;
            goto done_d;
        }

        bytes_orig += dsize;

        if (wbuf_push(&wb, decomp_buf, dsize) != 0) {
            result = COMPRESSION_ERR_OUTPUT;
            goto done_d;
        }
    }

    if (wbuf_flush(&wb) != 0)
        result = COMPRESSION_ERR_OUTPUT;

done_d:
    free(frame_buf);
    free(decomp_buf);
    fclose(in);
    close(out_fd);

    if (stats && result == COMPRESSION_OK) {
        stats_add_original_bytes(stats, bytes_orig);
        stats_add_written_bytes(stats, wb.total_written);
        stats_add_write_calls(stats, wb.write_calls);
        stats_timer_stop(&timer, stats);
    }

    return result;
}
