#include "ui/editor_mode.h"

#include <glib.h>

#include "app/controller.h"
#include "ui/canvas.h"

EditorMode *editor_mode_new(AppController *controller) {
    EditorMode *mode = g_new0(EditorMode, 1);
    mode->controller = controller;

    return mode;
}

void editor_mode_free(EditorMode *mode) {
    if (!mode)
        return;

    g_free(mode);
}

void editor_mode_enter(EditorMode *mode) {
    if (!mode || !mode->controller || !mode->controller->state)
        return;

    mode->controller->state->current_mode = APP_MODE_EDITOR;

    if (mode->controller->state->canvas) {
        ui_canvas_set_interactive(mode->controller->state->canvas, TRUE);
    }

    g_print("[MODE] Enter EDITOR mode\n");
}

void editor_mode_exit(EditorMode *mode) {
    if (!mode || !mode->controller || !mode->controller->state)
        return;

    if (mode->controller->state->canvas) {
        ui_canvas_clear_selection(mode->controller->state->canvas);
        ui_canvas_set_interactive(mode->controller->state->canvas, FALSE);
    }
}