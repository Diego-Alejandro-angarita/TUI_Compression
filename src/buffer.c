#include "buffer.h"

#include "secure_memory.h"

#include <stdlib.h>

void buffer_init(Buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
}

int buffer_reserve(Buffer *buffer, size_t capacity)
{
    uint8_t *new_data;

    if (buffer == NULL) {
        return -1;
    }

    if (capacity <= buffer->capacity) {
        return 0;
    }

    new_data = realloc(buffer->data, capacity);
    if (new_data == NULL) {
        return -1;
    }

    buffer->data = new_data;
    buffer->capacity = capacity;
    return 0;
}

void buffer_clear(Buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    if (buffer->data != NULL && buffer->capacity > 0) {
        secure_zero(buffer->data, buffer->capacity);
    }

    buffer->size = 0;
}

void buffer_free(Buffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    buffer_clear(buffer);
    free(buffer->data);
    buffer->data = NULL;
    buffer->capacity = 0;
}
