#include "secure_memory.h"

void secure_zero(void *ptr, size_t len)
{
    volatile unsigned char *cursor = (volatile unsigned char *)ptr;

    if (ptr == 0) {
        return;
    }

    while (len > 0) {
        *cursor = 0;
        cursor++;
        len--;
    }
}
