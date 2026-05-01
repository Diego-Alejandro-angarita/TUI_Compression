#include "app_state.h"

#include <stdlib.h>
#include <string.h>

void app_state_init(AppState *state)
{
    if (state == NULL) {
        return;
    }

    state->text_buffer = NULL;
    state->text_length = 0;
    state->current_path[0] = '\0';
    stats_init(&state->last_stats);
}

void app_state_destroy(AppState *state)
{
    if (state == NULL) {
        return;
    }

    free(state->text_buffer);
    state->text_buffer = NULL;
    state->text_length = 0;
    memset(state->current_path, 0, sizeof(state->current_path));
}
