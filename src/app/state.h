#ifndef APP_STATE_H
#define APP_STATE_H

#include <gtk/gtk.h>
#include "common/types.h"
#include "mode/manager.h"
#include "uart/port.h"

#define CONFIG_FILE_PATH "config/app.json"

typedef struct {
    GtkApplication *gtk_app;
    GtkWidget *window;
    GtkWidget *overlay;
    GtkWidget *canvas;
    AppConfig config;
    GPtrArray *layout_items;
    GPtrArray *value_items;
    AppMode current_mode;
    UartPort *uart_port;
} AppState;

AppState *app_state_new(void);
void app_state_free(AppState *state);
void app_state_set_background_path(AppState *state, const char *file_path);

#endif