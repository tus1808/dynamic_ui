#include "ui/read_mode.h"

#include <glib.h>
#include <gtk/gtk.h>

#include "app/controller.h"
#include "app/state.h"
#include "ui/canvas.h"

ReadMode *read_mode_new(AppController *controller)
{
  ReadMode *mode = g_new0(ReadMode, 1);
  mode->controller = controller;

  return mode;
}

void read_mode_free(ReadMode *mode)
{
  if (!mode)
    return;

  g_free(mode);
}

void read_mode_enter(ReadMode *mode)
{
  AppController *controller;

  if (!mode || !mode->controller)
    return;

  controller = mode->controller;

  if (!controller->state)
    return;

  controller->state->current_mode = APP_MODE_READ;

  if (controller->state->canvas)
  {
    ui_canvas_set_interactive(controller->state->canvas, FALSE);
  }

  if (controller->state->window)
  {
    GdkWindow *gdk_window = gtk_widget_get_window(controller->state->window);
    if (gdk_window)
    {
      gdk_window_set_cursor(gdk_window, NULL);
    }
  }
}

void read_mode_exit(ReadMode *mode)
{
  (void)mode;
}