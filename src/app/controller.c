#include "app/controller.h"
#include "config/layout_store.h"
#include "config/loader.h"
#include "ui/canvas.h"
#include "ui/window.h"
#include "ui/value_item.h"

#define CONFIG_FILE_PATH "config/app.json"
#define LAYOUT_FILE_PATH = "config/layout.json"

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
  if (!config_loader_load_app_config(CONFIG_FILE_PATH, &controller->state->config))
  {
    g_warning("Failed to load app config");
    return;
  }

  controller->state->layout_items = layout_store_load(controller->state->config.layout_file_path);
  if (!controller->state->layout_items)
  {
    g_warning("Failed to load layout items");
    return;
  }

  controller->state->canvas = ui_canvas_create();
  if (!ui_canvas_set_background(controller->state->canvas, controller->state->config.background))
  {
    g_warning("Failed to load background: %s", controller->state->config.background);
  }
  ui_canvas_render_items(controller->state->canvas, controller->state->layout_items);

  controller->state->window = ui_window_create(app, controller->state->canvas);
  if (controller->state->config.window_title)
  {
    gtk_window_set_title(GTK_WINDOW(controller->state->window), controller->state->config.window_title);
  }

  gtk_widget_show_all(controller->state->window);
}