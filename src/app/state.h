#ifndef APP_STATE_H
#define APP_STATE_H

#include <gtk/gtk.h>
#include "common/types.h"
#include "mode/manager.h"

typedef struct
{
  GtkApplication *gtk_app;
  GtkWidget *window;
  GtkWidget *canvas;

  AppConfig config;
  GPtrArray *layout_items;

  AppMode current_mode;
} AppState;

AppState *app_state_new(void);
void app_state_free(AppState *state);

#endif