#include "input/hotkey_manager.h"

#include <glib.h>
#include <gdk/gdkkeysyms.h>

#include "app/controller.h"
#include "app/state.h"
#include "config/layout_store.h"
#include "input/auth_manager.h"
#include "mode/manager.h"
#include "ui/editor_mode.h"

HotkeyManager *hotkey_manager_new(AppController *controller) {
    HotkeyManager *manager = g_new0(HotkeyManager, 1);
    manager->controller = controller;

    return manager;
}

void hotkey_manager_free(HotkeyManager *manager) {
    if (!manager)
        return;

    g_free(manager);
}

gboolean
hotkey_manager_handle_key_press(HotkeyManager *manager, GtkWidget *widget, GdkEventKey *event) {
    gboolean ctrl;
    gboolean shift;
    gboolean alt;

    (void)widget;

    if (!manager || !manager->controller || !event)
        return FALSE;

    ctrl = (event->state & GDK_CONTROL_MASK) != 0;
    shift = (event->state & GDK_SHIFT_MASK) != 0;
    alt = (event->state & GDK_MOD1_MASK) != 0;

    switch (mode_manager_is_editor_mode(manager->controller->mode_manager)) {
    case TRUE:
        if (event->keyval == GDK_KEY_Escape) {
            mode_manager_enter_read_mode(manager->controller->mode_manager);
            return TRUE;
        }

        if (ctrl && event->keyval == GDK_KEY_s) {
            if (manager->controller->state->layout_items) {
                layout_store_save(
                    manager->controller->state->config.layout_file_path,
                    manager->controller->state->layout_items
                );
                g_print("[LAYOUT] Saved\n");
            }
            return TRUE;
        }

        if (event->keyval == GDK_KEY_Delete) {
            editor_mode_delete_selected_item(manager->controller->editor_mode);
            return TRUE;
        }

        if (ctrl && event->keyval == GDK_KEY_n) {
            editor_mode_add_one_item(manager->controller->editor_mode);
            return TRUE;
        }

        if (ctrl && event->keyval == GDK_KEY_m) {
            editor_mode_add_many_items(manager->controller->editor_mode);
            return TRUE;
        }

        if (ctrl && event->keyval == GDK_KEY_b) {
            editor_mode_change_background(manager->controller->editor_mode);
            return TRUE;
        }

        if (ctrl && event->keyval == GDK_KEY_i) {
            editor_mode_show_info_box(manager->controller->editor_mode);
            return TRUE;
        }
        break;
    case FALSE:
        if (ctrl && shift && alt && event->keyval == GDK_KEY_F12) {
            auth_manager_request_editor_access(manager->controller->auth_manager);
            return TRUE;
        }
        break;
    default:
        break;
    }

    return FALSE;
}