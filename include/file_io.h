// include/file_io.h
#ifndef FILE_IO_H
#define FILE_IO_H

#include <stddef.h>

typedef enum {
    FILE_IO_OK          =  0,
    FILE_IO_ERR_PATH    = -1,
    FILE_IO_ERR_READ    = -2,
    FILE_IO_ERR_WRITE   = -3,
    FILE_IO_ERR_MEMORY  = -4
} FileIoResult;

FileIoResult file_io_read_all(const char *path, char **buffer, size_t *length);
FileIoResult file_io_write_all(const char *path, const char *buffer, size_t length);

#endif /* FILE_IO_H */

