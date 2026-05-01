#ifndef FILE_IO_H
#define FILE_IO_H

#include <stddef.h>

#include "stats.h"

typedef enum {
    FILE_IO_OK = 0,
    FILE_IO_ERR_OPEN,
    FILE_IO_ERR_READ,
    FILE_IO_ERR_WRITE,
    FILE_IO_ERR_MEMORY
} FileIoResult;

FileIoResult file_io_read_all(const char *path, char **buffer, size_t *length);
FileIoResult file_io_write_all(const char *path, const char *buffer, size_t length);
FileIoResult file_io_read_all_with_stats(
    const char *path,
    char **buffer,
    size_t *length,
    StatsReport *stats
);
FileIoResult file_io_write_all_with_stats(
    const char *path,
    const char *buffer,
    size_t length,
    StatsReport *stats
);

#endif
