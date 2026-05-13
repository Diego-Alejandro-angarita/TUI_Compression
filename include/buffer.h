#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} Buffer;

void buffer_init(Buffer *buffer);
int buffer_reserve(Buffer *buffer, size_t capacity);
void buffer_clear(Buffer *buffer);
void buffer_free(Buffer *buffer);

#endif /* BUFFER_H */
