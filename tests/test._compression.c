/*
 * tests/test_compression.c  —  Persona 2: suite de pruebas.
 *
 * Compilar y ejecutar (desde raíz del proyecto):
 *   gcc -Wall -Wextra -std=c11 -Iinclude \
 *       tests/test_compression.c src/compression.c src/stats.c \
 *       -lzstd -o build/test_compression
 *   ./build/test_compression
 *
 * O con el Makefile (cuando Persona 3 agregue el target):
 *   make test
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>

#include "compression.h"
#include "stats.h"

/* ── Mini framework ─────────────────────────────────────────────────────── */

static int g_run = 0, g_pass = 0, g_fail = 0;

#define T_BEGIN(name)  do { g_run++; printf("[TEST] %-54s ", (name)); fflush(stdout); } while(0)
#define T_PASS()       do { g_pass++; puts("PASS"); } while(0)
#define T_FAIL(why)    do { g_fail++; printf("FAIL — %s\n", (why)); } while(0)

/* ── Rutas temporales ───────────────────────────────────────────────────── */

static const char *F_ORIG   = "/tmp/_tc_orig.txt";
static const char *F_COMP   = "/tmp/_tc_comp.bin";
static const char *F_DECOMP = "/tmp/_tc_decomp.txt";

static void write_file(const char *path, const char *content)
{
    FILE *f = fopen(path, "wb");
    assert(f);
    fwrite(content, 1, strlen(content), f);
    fclose(f);
}

static char *read_file(const char *path, size_t *len_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = malloc((size_t)sz + 1);
    assert(buf);
    fread(buf, 1, (size_t)sz, f);
    buf[sz] = '\0';
    if (len_out) *len_out = (size_t)sz;
    fclose(f);
    return buf;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PRUEBAS
 * ═════════════════════════════════════════════════════════════════════════ */

/* 1 — round-trip básico: el descomprimido es igual al original */
static void test_roundtrip_basic(void)
{
    T_BEGIN("round-trip basico (compress + decompress)");

    const char *original =
        "Primera linea del archivo.\n"
        "Segunda linea con numeros: 1234567890.\n"
        "Tercera linea: ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
        "Cuarta linea.\n";

    write_file(F_ORIG, original);

    StatsReport stats; stats_init(&stats);

    CompressionResult r1 = compression_compress_file(F_ORIG, F_COMP, &stats);
    if (r1 != COMPRESSION_OK) { T_FAIL("compress fallo"); return; }

    CompressionResult r2 = compression_decompress_file(F_COMP, F_DECOMP, &stats);
    if (r2 != COMPRESSION_OK) { T_FAIL("decompress fallo"); return; }

    size_t decomp_len;
    char *decomp = read_file(F_DECOMP, &decomp_len);
    if (!decomp) { T_FAIL("no se pudo leer el archivo descomprimido"); return; }

    size_t orig_len = strlen(original);
    int equal = (decomp_len == orig_len && memcmp(original, decomp, orig_len) == 0);
    free(decomp);

    if (equal) T_PASS(); else T_FAIL("contenido distinto al original");
}

/* 2 — archivo con línea vacía en el medio */
static void test_roundtrip_empty_line(void)
{
    T_BEGIN("round-trip con linea vacia en el medio");

    const char *original = "Linea uno.\n\nLinea tres.\n";
    write_file(F_ORIG, original);

    compression_compress_file(F_ORIG, F_COMP, NULL);
    compression_decompress_file(F_COMP, F_DECOMP, NULL);

    size_t len;
    char *decomp = read_file(F_DECOMP, &len);
    int ok = decomp && len == strlen(original) &&
             memcmp(original, decomp, len) == 0;
    free(decomp);

    if (ok) T_PASS(); else T_FAIL("fallo con linea vacia");
}

/* 3 — stats: bytes_original y bytes_compressed se rellenan */
static void test_stats_bytes(void)
{
    T_BEGIN("stats: bytes_original y bytes_compressed > 0");

    write_file(F_ORIG, "Contenido para medir estadisticas.\n");

    StatsReport stats; stats_init(&stats);
    compression_compress_file(F_ORIG, F_COMP, &stats);

    if (stats.bytes_original == 0)   { T_FAIL("bytes_original == 0"); return; }
    if (stats.bytes_compressed == 0) { T_FAIL("bytes_compressed == 0"); return; }
    T_PASS();
}

/* 4 — stats: write_calls > 0 */
static void test_stats_write_calls(void)
{
    T_BEGIN("stats: write_calls > 0 tras compresion");

    write_file(F_ORIG, "Linea de prueba para write_calls.\n");

    StatsReport stats; stats_init(&stats);
    compression_compress_file(F_ORIG, F_COMP, &stats);

    if (stats.write_calls == 0) T_FAIL("write_calls sigue en 0");
    else T_PASS();
}

/* 5 — stats: wall_clock_ms >= 0 */
static void test_stats_wall_clock(void)
{
    T_BEGIN("stats: wall_clock_ms >= 0");

    write_file(F_ORIG, "Prueba de tiempo.\n");

    StatsReport stats; stats_init(&stats);
    compression_compress_file(F_ORIG, F_COMP, &stats);

    if (stats.wall_clock_ms < 0.0) T_FAIL("wall_clock_ms es negativo");
    else T_PASS();
}

/* 6 — stats_finalize calcula compression_ratio */
static void test_stats_finalize_ratio(void)
{
    T_BEGIN("stats_finalize: compression_ratio calculado");

    write_file(F_ORIG, "Texto repetido para calcular ratio de compresion.\n");

    StatsReport stats; stats_init(&stats);
    compression_compress_file(F_ORIG, F_COMP, &stats);
    stats_finalize(&stats);

    if (stats.compression_ratio <= 0.0)
        T_FAIL("compression_ratio <= 0 tras stats_finalize");
    else
        T_PASS();
}

/* 7 — input_path NULL devuelve COMPRESSION_ERR_INPUT */
static void test_null_input_compress(void)
{
    T_BEGIN("compress: input NULL => COMPRESSION_ERR_INPUT");

    CompressionResult r = compression_compress_file(NULL, F_COMP, NULL);
    if (r == COMPRESSION_ERR_INPUT) T_PASS();
    else T_FAIL("no devolvio COMPRESSION_ERR_INPUT");
}

/* 8 — output_path NULL devuelve COMPRESSION_ERR_INPUT */
static void test_null_output_compress(void)
{
    T_BEGIN("compress: output NULL => COMPRESSION_ERR_INPUT");

    CompressionResult r = compression_compress_file(F_ORIG, NULL, NULL);
    if (r == COMPRESSION_ERR_INPUT) T_PASS();
    else T_FAIL("no devolvio COMPRESSION_ERR_INPUT");
}

/* 9 — archivo fuente que no existe devuelve COMPRESSION_ERR_INPUT */
static void test_missing_input_file(void)
{
    T_BEGIN("compress: archivo inexistente => COMPRESSION_ERR_INPUT");

    CompressionResult r = compression_compress_file(
        "/tmp/no_existe_nunca_12345.txt", F_COMP, NULL);
    if (r == COMPRESSION_ERR_INPUT) T_PASS();
    else T_FAIL("no devolvio COMPRESSION_ERR_INPUT");
}

/* 10 — destino en ruta inaccesible devuelve COMPRESSION_ERR_OUTPUT */
static void test_bad_output_path(void)
{
    T_BEGIN("compress: ruta destino invalida => COMPRESSION_ERR_OUTPUT");

    write_file(F_ORIG, "contenido.\n");
    CompressionResult r = compression_compress_file(
        F_ORIG, "/ruta/que/no/existe/archivo.bin", NULL);
    if (r == COMPRESSION_ERR_OUTPUT) T_PASS();
    else T_FAIL("no devolvio COMPRESSION_ERR_OUTPUT");
}

/* 11 — round-trip con NULL en stats (no debe crashear) */
static void test_roundtrip_null_stats(void)
{
    T_BEGIN("round-trip con stats=NULL no crashea");

    write_file(F_ORIG, "Prueba sin estructura de estadisticas.\n");

    CompressionResult r1 = compression_compress_file(F_ORIG, F_COMP, NULL);
    CompressionResult r2 = compression_decompress_file(F_COMP, F_DECOMP, NULL);

    if (r1 == COMPRESSION_OK && r2 == COMPRESSION_OK) T_PASS();
    else T_FAIL("fallo con stats=NULL");
}

/* 12 — archivo con una sola línea sin '\n' al final */
static void test_roundtrip_no_trailing_newline(void)
{
    T_BEGIN("round-trip archivo sin '\\n' al final");

    const char *original = "Linea sin salto de linea al final";
    write_file(F_ORIG, original);

    compression_compress_file(F_ORIG, F_COMP, NULL);
    compression_decompress_file(F_COMP, F_DECOMP, NULL);

    size_t len;
    char *decomp = read_file(F_DECOMP, &len);
    int ok = decomp && len == strlen(original) &&
             memcmp(original, decomp, len) == 0;
    free(decomp);

    if (ok) T_PASS(); else T_FAIL("contenido distinto para archivo sin newline");
}

/* ── Limpieza ───────────────────────────────────────────────────────────── */

static void cleanup(void)
{
    unlink(F_ORIG);
    unlink(F_COMP);
    unlink(F_DECOMP);
}

/* ── main ───────────────────────────────────────────────────────────────── */

int main(void)
{
    puts("=== Tests: compression.c (Persona 2) ===\n");

    test_roundtrip_basic();
    test_roundtrip_empty_line();
    test_stats_bytes();
    test_stats_write_calls();
    test_stats_wall_clock();
    test_stats_finalize_ratio();
    test_null_input_compress();
    test_null_output_compress();
    test_missing_input_file();
    test_bad_output_path();
    test_roundtrip_null_stats();
    test_roundtrip_no_trailing_newline();

    cleanup();

    printf("\n=== %d/%d pasaron", g_pass, g_run);
    if (g_fail > 0) printf(", %d FALLARON", g_fail);
    puts(" ===\n");

    return (g_fail == 0) ? 0 : 1;
}