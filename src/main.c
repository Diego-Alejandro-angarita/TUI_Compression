#include "app_state.h"
#include "tui.h"

int main(void)
{
    AppState state;
    int result;

    app_state_init(&state);
    result = tui_run(&state);
    app_state_destroy(&state);

    return result;
}

