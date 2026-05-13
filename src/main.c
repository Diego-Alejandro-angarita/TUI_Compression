#include "io_pipeline.h"

#include <stdio.h>

int main(void)
{
    if (io_pipeline_demo() != IO_PIPELINE_OK) {
        fprintf(stderr, "Failed to initialize secure compression pipeline structure.\n");
        return 1;
    }

    printf("Secure compression pipeline structure initialized successfully.\n");
    return 0;
}
