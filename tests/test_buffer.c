#include "buffer.h"

int main(void)
{
    Buffer buffer;

    buffer_init(&buffer);
    if (buffer_reserve(&buffer, 16) != 0) {
        return 1;
    }

    buffer_clear(&buffer);
    buffer_free(&buffer);
    return 0;
}
