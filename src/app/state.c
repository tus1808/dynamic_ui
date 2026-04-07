#include "app/state.h"
#include <glib.h>

AppState *app_state_new(void)
{
  AppState *state = g_new0(AppState, 1);
  state->background_path = g_strdup("assets/background.png");
  state->window_width = 0;
  state->window_height = 0;

  return state;
}

void app_state_free(AppState *state)
{
  if (!state)
    return;

  g_free(state->background_path);
  g_free(state);
}

void app_state_set_background_path(AppState *state, const char *path)
{
  if (!state || path)
    return;

  g_free(state->background_path);
  state->background_path = g_strdup(path);
}