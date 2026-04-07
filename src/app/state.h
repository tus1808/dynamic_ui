#ifndef APP_STATE_H
#define APP_STATE_H

#include <gtk/gtk.h>

typedef struct
{
  GtkApplication *gtk_app;
  GtkWidget *window;
  GtkWidget *canvas;

  char *background_path;

  int window_width;
  int window_height;
} AppState;

AppState *app_state_new(void);
void app_state_free(AppState *state);
void app_state_set_background_path(AppState *state, const char *path);

#endif