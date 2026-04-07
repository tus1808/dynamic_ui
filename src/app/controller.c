#include "app/controller.h"
#include "ui/canvas.h"
#include "ui/window.h"
#include <glib.h>

AppController *app_controller_new(void)
{
  AppController *controller = g_new0(AppController, 1);
  controller->state = app_state_new();

  return controller;
}

void app_controller_free(AppController *controller)
{
  if (!controller)
    return;

  app_state_free(controller->state);
  g_free(controller);
}

void app_controller_activate(AppController *controller, GtkApplication *app)
{
  if (!controller || !controller->state)
    return;

  controller->state->gtk_app = app;

  controller->state->canvas = ui_canvas_create();
  if (!ui_canvas_set_background(controller->state->canvas, controller->state->background_path))
  {
    g_warning("Failed to load background: %s", controller->state->background_path);
  }

  controller->state->window = ui_window_create(app, controller->state->canvas);

  gtk_widget_show_all(controller->state->window);
}