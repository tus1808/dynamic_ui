#include "app/state.h"

#include <glib.h>

#include "config/loader.h"

AppState *app_state_new(void) {
    AppState *state = g_new0(AppState, 1);

    state->gtk_app = NULL;
    state->window = NULL;
    state->overlay = NULL;
    state->canvas = NULL;
    state->layout_items = g_ptr_array_new();
    state->value_items = g_ptr_array_new();
    state->current_mode = APP_MODE_READ;
    state->uart_port = NULL;

    return state;
}

void app_state_free(AppState *state) {
    if (!state)
        return;

    if (state->uart_port) {
        uart_port_stop(state->uart_port);
        uart_port_free(state->uart_port);
        state->uart_port = NULL;
    }

    if (state->layout_items) {
        g_ptr_array_free(state->layout_items, TRUE);
        state->layout_items = NULL;
    }

    if (state->value_items) {
        g_ptr_array_free(state->value_items, TRUE);
        state->value_items = NULL;
    }

    config_loader_free_app_config(&state->config);
    g_free(state);
}

void app_state_set_background_path(AppState *state, const char *file_path) {
    if (!state)
        return;

    g_free(state->config.background);
    state->config.background = g_strdup(file_path);
}