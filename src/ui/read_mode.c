#include "ui/read_mode.h"

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "app/controller.h"
#include "app/state.h"
#include "ui/canvas.h"

ReadMode *read_mode_new(AppController *controller) {
    ReadMode *mode = g_new0(ReadMode, 1);
    mode->controller = controller;

    return mode;
}

void read_mode_free(ReadMode *mode) {
    if (!mode)
        return;

    g_free(mode);
}

void read_mode_enter(ReadMode *mode) {
    if (!mode || !mode->controller)
        return;

    if (!mode->controller->state)
        return;

    mode->controller->state->current_mode = APP_MODE_READ;

    if (mode->controller->state->canvas) {
        ui_canvas_set_interactive(mode->controller->state->canvas, FALSE);
    }

    if (mode->controller->state->window) {
        GdkWindow *gdk_window = gtk_widget_get_window(mode->controller->state->window);
        if (gdk_window) {
            gdk_window_set_cursor(gdk_window, NULL);
        }
    }

    g_print("[MODE] Enter READ mode\n");
}

void read_mode_exit(ReadMode *mode) { (void)mode; }