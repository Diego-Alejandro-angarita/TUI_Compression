#define _POSIX_C_SOURCE 200809L

#include "file_io.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static int ensure_capacity(char **buffer, size_t *capacity, size_t needed)
{
    if (needed <= *capacity) {
        return 0;
    }

    size_t new_capacity = (*capacity == 0) ? 4096u : *capacity;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2u) {
            return -1;
        }
        new_capacity *= 2u;
    }

    char *new_buffer = realloc(*buffer, new_capacity + 1u);
    if (new_buffer == NULL) {
        return -1;
    }

    *buffer = new_buffer;
    *capacity = new_capacity;
    return 0;
}

FileIoResult file_io_read_all(const char *path, char **buffer, size_t *length)
{
    return file_io_read_all_with_stats(path, buffer, length, NULL);
}

FileIoResult file_io_read_all_with_stats(
    const char *path,
    char **buffer,
    size_t *length,
    StatsReport *stats)
{
    if (buffer == NULL || length == NULL) {
        return FILE_IO_ERR_READ;
    }

    *buffer = NULL;
    *length = 0;

    if (path == NULL) {
        return FILE_IO_ERR_OPEN;
    }

    StatsTimer timer;
    stats_timer_start(&timer);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return FILE_IO_ERR_OPEN;
    }

    size_t capacity = 4096u;
    struct stat info;
    if (fstat(fd, &info) == 0 && S_ISREG(info.st_mode) && info.st_size > 0) {
        capacity = (size_t)info.st_size;
    }

    char *data = malloc(capacity + 1u);
    if (data == NULL) {
        close(fd);
        return FILE_IO_ERR_MEMORY;
    }

    size_t used = 0;
    for (;;) {
        if (ensure_capacity(&data, &capacity, used + 4096u) != 0) {
            free(data);
            close(fd);
            return FILE_IO_ERR_MEMORY;
        }

        ssize_t read_count = read(fd, data + used, capacity - used);
        if (read_count < 0) {
            if (errno == EINTR) {
                continue;
            }
            free(data);
            close(fd);
            return FILE_IO_ERR_READ;
        }
        if (read_count == 0) {
            break;
        }

        used += (size_t)read_count;
    }

    if (close(fd) != 0) {
        free(data);
        return FILE_IO_ERR_READ;
    }

    data[used] = '\0';
    *buffer = data;
    *length = used;

    stats_add_original_bytes(stats, (uint64_t)used);
    stats_timer_stop(&timer, stats);
    return FILE_IO_OK;
}

FileIoResult file_io_write_all(const char *path, const char *buffer, size_t length)
{
    return file_io_write_all_with_stats(path, buffer, length, NULL);
}

FileIoResult file_io_write_all_with_stats(
    const char *path,
    const char *buffer,
    size_t length,
    StatsReport *stats)
{
    if (path == NULL) {
        return FILE_IO_ERR_OPEN;
    }
    if (buffer == NULL && length > 0) {
        return FILE_IO_ERR_WRITE;
    }

    StatsTimer timer;
    stats_timer_start(&timer);

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return FILE_IO_ERR_OPEN;
    }

    size_t offset = 0;
    uint64_t write_calls = 0;
    uint64_t bytes_written = 0;

    while (offset < length) {
        ssize_t write_count = write(fd, buffer + offset, length - offset);
        write_calls++;

        if (write_count < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(fd);
            return FILE_IO_ERR_WRITE;
        }
        if (write_count == 0) {
            close(fd);
            return FILE_IO_ERR_WRITE;
        }

        offset += (size_t)write_count;
        bytes_written += (uint64_t)write_count;
    }

    if (close(fd) != 0) {
        return FILE_IO_ERR_WRITE;
    }

    stats_add_written_bytes(stats, bytes_written);
    stats_add_write_calls(stats, write_calls);
    stats_timer_stop(&timer, stats);
    return FILE_IO_OK;
}
