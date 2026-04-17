#include "app/controller.h"

#include <glib.h>

#include "common/contants.h"
#include "config/layout_store.h"
#include "config/loader.h"
#include "mode/manager.h"
#include "uart/port.h"
#include "ui/canvas.h"
#include "ui/overlay.h"
#include "ui/window.h"

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

static void on_uart_frame(const guint8 *frame, gsize frame_size, gpointer user_data) {
    AppController *controller = user_data;

    if (!controller || !controller->data_dispatcher)
        return;

    data_dispatcher_submit_frame(controller->data_dispatcher, frame, frame_size);
}

void app_uart_init(AppController *controller) {
    AppState *state;

    if (!controller || !controller->state)
        return;

    state = controller->state;

    if (state->uart_port) {
        uart_port_stop(state->uart_port);
        uart_port_free(state->uart_port);
        state->uart_port = NULL;
    }

    state->uart_port = uart_port_new(9600, on_uart_frame, controller);
    if (!state->uart_port) {
        g_printerr("Failed to create UART port object.\n");
        return;
    }

    if (!uart_port_open(state->uart_port, UART_DEFAULT_DEVICE)) {
        g_printerr("Failed to open %s\n", UART_DEFAULT_DEVICE);
        uart_port_free(state->uart_port);
        state->uart_port = NULL;

        return;
    }

    g_print("Using UART port: %s\n", UART_DEFAULT_DEVICE);

    if (!uart_port_start(state->uart_port)) {
        g_printerr("Failed to start UART read thread.\n");
        uart_port_free(state->uart_port);
        state->uart_port = NULL;

        return;
    }
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
    controller->data_dispatcher = data_dispatcher_new(controller->state);

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
    data_dispatcher_free(controller->data_dispatcher);
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
    if (!controller->state->canvas) {
        g_warning("Failed to create canvas");
        return;
    }

    controller->state->overlay = ui_overlay_create(controller->state->canvas);
    if (!controller->state->overlay) {
        g_warning("Failed to create overlay");
        return;
    }
    if (!ui_overlay_set_background(
            controller->state->overlay,
            controller->state->config.background
        )) {
        g_warning("Failed to load background: %s", controller->state->config.background);
    }

    controller->state->layout_items = layout_store_load(controller->state->config.layout_file_path);
    if (!controller->state->layout_items) {
        g_warning("Failed to load layout items");
        return;
    }
    ui_canvas_render_items(
        controller->state->canvas,
        controller->state->layout_items,
        controller->state->value_items
    );

    controller->state->window = ui_window_create(app, controller->state->overlay);
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
    ui_overlay_set_toolbar(controller->state->overlay, toolbar_container);

    app_controller_load_css(controller->state->config.css_file_path);

    gtk_widget_show_all(controller->state->window);
    mode_manager_enter_read_mode(controller->mode_manager);

    app_uart_init(controller);
}