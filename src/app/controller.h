#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "app/state.h"

#include <gtk/gtk.h>

#include "mode/manager.h"
#include "ui/read_mode.h"
#include "ui/editor_mode.h"

typedef struct _AppController
{
  AppState *state;
  ModeManager *mode_manager;
  ReadMode *read_mode;
  EditorMode *editor_mode;
} AppController;

AppController *app_controller_new(void);
void app_controller_free(AppController *controller);
void app_controller_activate(AppController *controller, GtkApplication *app);

#endif