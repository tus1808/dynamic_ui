#include "mode/manager.h"

#include <glib.h>

#include "app/controller.h"
#include "ui/editor_toolbar.h"
#include "ui/read_mode.h"

ModeManager *mode_manager_new(AppController *controller) {
    ModeManager *manager = g_new0(ModeManager, 1);
    manager->controller = controller;
    manager->current_mode = APP_MODE_READ;

    return manager;
}

void mode_manager_free(ModeManager *manager) {
    if (!manager)
        return;

    g_free(manager);
}

AppMode mode_manager_get_mode(const ModeManager *manager) {
    if (!manager)
        return APP_MODE_READ;

    return manager->current_mode;
}

gboolean mode_manager_is_read_mode(const ModeManager *manager) {
    return mode_manager_get_mode(manager) == APP_MODE_READ;
}

gboolean mode_manager_is_editor_mode(const ModeManager *manager) {
    return mode_manager_get_mode(manager) == APP_MODE_EDITOR;
}

void mode_manager_set_mode(ModeManager *manager, AppMode mode) {
    if (!manager || !manager->controller)
        return;

    if (manager->current_mode == mode)
        return;

    switch (manager->current_mode) {
    case APP_MODE_READ:
        if (manager->controller->read_mode) {
            read_mode_exit(manager->controller->read_mode);
        }
        break;

    case APP_MODE_EDITOR:
        if (manager->controller->editor_mode) {
            editor_mode_exit(manager->controller->editor_mode);
        }
        break;

    default:
        break;
    }

    manager->current_mode = mode;

    switch (manager->current_mode) {
    case APP_MODE_READ:
        if (manager->controller->read_mode) {
            read_mode_enter(manager->controller->read_mode);
        }
        break;

    case APP_MODE_EDITOR:
        if (manager->controller->editor_mode) {
            editor_mode_enter(manager->controller->editor_mode);
        }
        break;

    default:
        break;
    }
}

void mode_manager_enter_read_mode(ModeManager *manager) {
    mode_manager_set_mode(manager, APP_MODE_READ);
    editor_toolbar_hide(manager->controller->editor_toolbar);
}

void mode_manager_enter_editor_mode(ModeManager *manager) {
    mode_manager_set_mode(manager, APP_MODE_EDITOR);
    editor_toolbar_show(manager->controller->editor_toolbar);
}