// src/file_io.c
#include "file_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DATA_DIR      "data/"
#define DATA_DIR_LEN  5
#define MAX_PATH      512

static int build_safe_path(const char *name, char *out, size_t out_sz)
{
    if (!name || !*name)
        return 0;
    if (strstr(name, "..") || strchr(name, '/') || strchr(name, '\\'))
        return 0;
    int n = snprintf(out, out_sz, "%s%s", DATA_DIR, name);
    if (n < 0 || (size_t)n >= out_sz)
        return 0;
    return 1;
}

static void ensure_data_dir(void)
{
    struct stat st;
    if (stat(DATA_DIR, &st) != 0)
        mkdir(DATA_DIR, 0755);
}

FileIoResult file_io_read_all(const char *path, char **buffer, size_t *length)
{
    if (!path || !buffer || !length)
        return FILE_IO_ERR_PATH;

    char safe[MAX_PATH];
    if (!build_safe_path(path, safe, sizeof(safe)))
        return FILE_IO_ERR_PATH;

    FILE *f = fopen(safe, "rb");
    if (!f)
        return FILE_IO_ERR_READ;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);

    if (sz < 0) { fclose(f); return FILE_IO_ERR_READ; }

    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return FILE_IO_ERR_MEMORY; }

    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);

    buf[rd] = '\0';
    *buffer = buf;
    *length = rd;
    return FILE_IO_OK;
}

FileIoResult file_io_write_all(const char *path, const char *buffer, size_t length)
{
    if (!path || !buffer)
        return FILE_IO_ERR_PATH;

    char safe[MAX_PATH];
    if (!build_safe_path(path, safe, sizeof(safe)))
        return FILE_IO_ERR_PATH;

    ensure_data_dir();

    FILE *f = fopen(safe, "wb");
    if (!f)
        return FILE_IO_ERR_WRITE;

    size_t wr = fwrite(buffer, 1, length, f);
    fclose(f);

    return (wr == length) ? FILE_IO_OK : FILE_IO_ERR_WRITE;
}