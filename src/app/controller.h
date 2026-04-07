#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <gtk/gtk.h>
#include "app/state.h"

typedef struct
{
  AppState *state;
} AppController;

AppController *app_controller_new(void);
void app_controller_free(AppController *controller);
void app_controller_activate(AppController *controller, GtkApplication *app);

#endif