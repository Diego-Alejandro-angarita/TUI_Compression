#include "file_io.h"

FileIoResult file_io_read_all(const char *path, char **buffer, size_t *length)
{
    (void)path;
    (void)buffer;
    (void)length;

    return FILE_IO_ERR_READ;
}

FileIoResult file_io_write_all(const char *path, const char *buffer, size_t length)
{
    (void)path;
    (void)buffer;
    (void)length;

    return FILE_IO_ERR_WRITE;
}

