#define _POSIX_C_SOURCE 200809L

#include "file_io.h"
#include "stats.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_run = 0;
static int g_pass = 0;
static int g_fail = 0;

#define T_BEGIN(name) do { g_run++; printf("[TEST] %-54s ", (name)); fflush(stdout); } while (0)
#define T_PASS() do { g_pass++; puts("PASS"); } while (0)
#define T_FAIL(why) do { g_fail++; printf("FAIL - %s\n", (why)); } while (0)

static const char *F_TEXT = "/tmp/_fio_text.txt";
static const char *F_EMPTY = "/tmp/_fio_empty.txt";

static void test_write_then_read_text(void)
{
    T_BEGIN("file_io: write_all + read_all conserva contenido");

    const char *text = "Linea uno\nLinea dos\nLinea tres sin acento\n";
    FileIoResult wr = file_io_write_all(F_TEXT, text, strlen(text));
    if (wr != FILE_IO_OK) {
        T_FAIL("write_all fallo");
        return;
    }

    char *read_buffer = NULL;
    size_t read_length = 0;
    FileIoResult rd = file_io_read_all(F_TEXT, &read_buffer, &read_length);
    if (rd != FILE_IO_OK) {
        T_FAIL("read_all fallo");
        return;
    }

    int ok = read_length == strlen(text)
        && memcmp(read_buffer, text, read_length) == 0
        && read_buffer[read_length] == '\0';
    free(read_buffer);

    if (ok) {
        T_PASS();
    } else {
        T_FAIL("contenido leido distinto");
    }
}

static void test_binary_content_with_nul(void)
{
    T_BEGIN("file_io: conserva bytes NUL internos");

    const char content[] = { 'A', '\0', 'B', '\n', 'C' };
    FileIoResult wr = file_io_write_all(F_TEXT, content, sizeof(content));
    if (wr != FILE_IO_OK) {
        T_FAIL("write_all fallo");
        return;
    }

    char *read_buffer = NULL;
    size_t read_length = 0;
    FileIoResult rd = file_io_read_all(F_TEXT, &read_buffer, &read_length);
    if (rd != FILE_IO_OK) {
        T_FAIL("read_all fallo");
        return;
    }

    int ok = read_length == sizeof(content)
        && memcmp(read_buffer, content, sizeof(content)) == 0;
    free(read_buffer);

    if (ok) {
        T_PASS();
    } else {
        T_FAIL("bytes binarios distintos");
    }
}

static void test_empty_file(void)
{
    T_BEGIN("file_io: archivo vacio");

    FileIoResult wr = file_io_write_all(F_EMPTY, "", 0);
    if (wr != FILE_IO_OK) {
        T_FAIL("write_all vacio fallo");
        return;
    }

    char *read_buffer = NULL;
    size_t read_length = 1;
    FileIoResult rd = file_io_read_all(F_EMPTY, &read_buffer, &read_length);
    if (rd != FILE_IO_OK) {
        T_FAIL("read_all vacio fallo");
        return;
    }

    int ok = read_length == 0 && read_buffer != NULL && read_buffer[0] == '\0';
    free(read_buffer);

    if (ok) {
        T_PASS();
    } else {
        T_FAIL("archivo vacio mal representado");
    }
}

static void test_missing_file(void)
{
    T_BEGIN("file_io: archivo inexistente => FILE_IO_ERR_OPEN");

    char *read_buffer = (char *)1;
    size_t read_length = 99;
    FileIoResult rd = file_io_read_all(
        "/tmp/no_existe_file_io_12345.txt",
        &read_buffer,
        &read_length
    );

    if (rd == FILE_IO_ERR_OPEN && read_buffer == NULL && read_length == 0) {
        T_PASS();
    } else {
        T_FAIL("error inesperado para archivo inexistente");
    }
}

static void test_stats_on_read_and_write(void)
{
    T_BEGIN("file_io: stats de lectura y escritura");

    const char *text = "Contenido para medir file_io.\n";
    StatsReport stats;
    stats_init(&stats);

    FileIoResult wr = file_io_write_all_with_stats(
        F_TEXT,
        text,
        strlen(text),
        &stats
    );
    if (wr != FILE_IO_OK) {
        T_FAIL("write_all_with_stats fallo");
        return;
    }

    char *read_buffer = NULL;
    size_t read_length = 0;
    FileIoResult rd = file_io_read_all_with_stats(
        F_TEXT,
        &read_buffer,
        &read_length,
        &stats
    );
    free(read_buffer);

    if (rd != FILE_IO_OK) {
        T_FAIL("read_all_with_stats fallo");
        return;
    }
    if (stats.bytes_written != strlen(text)) {
        T_FAIL("bytes_written incorrecto");
        return;
    }
    if (stats.write_calls == 0) {
        T_FAIL("write_calls no aumento");
        return;
    }
    if (stats.bytes_original != read_length) {
        T_FAIL("bytes_original de lectura incorrecto");
        return;
    }
    if (stats.wall_clock_ms < 0.0 || stats.cpu_user_ms < 0.0
        || stats.cpu_sys_ms < 0.0) {
        T_FAIL("tiempos negativos");
        return;
    }

    T_PASS();
}

static void cleanup(void)
{
    unlink(F_TEXT);
    unlink(F_EMPTY);
}

int main(void)
{
    puts("=== Tests: file_io.c (Persona 3) ===\n");

    test_write_then_read_text();
    test_binary_content_with_nul();
    test_empty_file();
    test_missing_file();
    test_stats_on_read_and_write();
    cleanup();

    printf("\n=== %d/%d pasaron", g_pass, g_run);
    if (g_fail > 0) {
        printf(", %d FALLARON", g_fail);
    }
    puts(" ===\n");

    return (g_fail == 0) ? 0 : 1;
}
