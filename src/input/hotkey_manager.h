#ifndef HOTKEY_MANAGER_H
#define HOTKEY_MANAGER_H

#include <gtk/gtk.h>

typedef struct _AppController AppController;

typedef struct {
    AppController *controller;
} HotkeyManager;

HotkeyManager *hotkey_manager_new(AppController *controller);
void hotkey_manager_free(HotkeyManager *manager);

gboolean
hotkey_manager_handle_key_press(HotkeyManager *manager, GtkWidget *widget, GdkEventKey *event);

#endif