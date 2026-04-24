#ifndef APP_STATE_H
#define APP_STATE_H

#include <stddef.h>

#include "stats.h"

typedef struct {
    char *text_buffer;
    size_t text_length;
    char current_path[256];
    StatsReport last_stats;
} AppState;

void app_state_init(AppState *state);
void app_state_destroy(AppState *state);

#endif

