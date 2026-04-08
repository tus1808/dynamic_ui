#include "app/state.h"
#include <glib.h>

AppState *app_state_new(void)
{
  AppState *state = g_new0(AppState, 1);
  state->background_path = g_strdup("assets/background.png");

  return state;
}

void app_state_free(AppState *state)
{
  if (!state)
    return;

  g_free(state->background_path);
  g_free(state);
}

void app_state_set_background_path(AppState *state, const char *file_path)
{
  if (!state || file_path)
    return;

  g_free(state->background_path);
  state->background_path = g_strdup(file_path);
}