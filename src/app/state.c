#include "app/state.h"

#include <glib.h>

#include "config/loader.h"

AppState *app_state_new(void)
{
  AppState *state = g_new0(AppState, 1);
  state->current_mode = APP_MODE_READ;

  return state;
}

void app_state_free(AppState *state)
{
  if (!state)
    return;

  if (state->layout_items)
  {
    g_ptr_array_free(state->layout_items, TRUE);
    state->layout_items = NULL;
  }

  config_loader_free_app_config(&state->config);

  g_free(state);
}