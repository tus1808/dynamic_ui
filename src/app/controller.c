#include "app/controller.h"

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "config/layout_store.h"
#include "config/loader.h"
#include "ui/canvas.h"
#include "ui/window.h"

#define CONFIG_FILE_PATH "config/app.json"

static void app_controller_load_css(const char *file_path) {
    GtkCssProvider *provider;
    GdkScreen *screen;
    GError *error = NULL;

    if (!file_path)
        return;

    provider = gtk_css_provider_new();
    screen = gdk_screen_get_default();

    if (!gtk_css_provider_load_from_path(provider, file_path, &error)) {
        g_warning(
            "Failed to load CSS '%s': %s",
            file_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);
        g_object_unref(provider);

        return;
    }

    g_print("[CSS] Loaded: %s\n", file_path);

    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    AppController *controller = user_data;

    if (!controller || !controller->hotkey_manager)
        return FALSE;

    return hotkey_manager_handle_key_press(controller->hotkey_manager, widget, event);
}

AppController *app_controller_new(void) {
    AppController *controller = g_new0(AppController, 1);

    controller->state = app_state_new();
    controller->read_mode = read_mode_new(controller);
    controller->editor_mode = editor_mode_new(controller);
    controller->editor_toolbar = NULL;
    controller->mode_manager = mode_manager_new(controller);
    controller->auth_manager = auth_manager_new(controller);
    controller->hotkey_manager = hotkey_manager_new(controller);

    return controller;
}

void app_controller_free(AppController *controller) {
    if (!controller)
        return;

    mode_manager_free(controller->mode_manager);
    read_mode_free(controller->read_mode);
    editor_mode_free(controller->editor_mode);
    editor_toolbar_free(controller->editor_toolbar);
    auth_manager_free(controller->auth_manager);
    hotkey_manager_free(controller->hotkey_manager);
    app_state_free(controller->state);

    g_free(controller);
}

void app_controller_activate(AppController *controller, GtkApplication *app) {
    if (!controller || !controller->state)
        return;

    controller->state->gtk_app = app;
    if (!config_loader_load_app_config(CONFIG_FILE_PATH, &controller->state->config)) {
        g_warning("Failed to load app config");
        return;
    }

    controller->state->canvas = ui_canvas_create();
    if (!ui_canvas_set_background(
            controller->state->canvas,
            controller->state->config.background
        )) {
        g_warning("Failed to load background: %s", controller->state->config.background);
    }

    controller->state->layout_items = layout_store_load(controller->state->config.layout_file_path);
    if (!controller->state->layout_items) {
        g_warning("Failed to load layout items");
        return;
    }
    ui_canvas_render_items(controller->state->canvas, controller->state->layout_items);

    controller->state->window = ui_window_create(app, controller->state->canvas);
    if (controller->state->config.window_title) {
        gtk_window_set_title(
            GTK_WINDOW(controller->state->window),
            controller->state->config.window_title
        );
    }
    gtk_widget_add_events(controller->state->window, GDK_KEY_PRESS_MASK);

    g_signal_connect(
        controller->state->window,
        "key-press-event",
        G_CALLBACK(on_key_press),
        controller
    );

    controller->editor_toolbar = editor_toolbar_new(controller);
    GtkWidget *toolbar_container = editor_toolbar_get_widget(controller->editor_toolbar);
    gtk_fixed_put(GTK_FIXED(controller->state->canvas), toolbar_container, 800, 12);

    app_controller_load_css(controller->state->config.css_file_path);

    gtk_widget_show_all(controller->state->window);
    mode_manager_enter_read_mode(controller->mode_manager);
}