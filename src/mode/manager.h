#ifndef MANAGER_H
#define MANAGER_H

#include <glib.h>

typedef struct _AppController AppController;

typedef enum
{
  APP_MODE_READ = 0,
  APP_MODE_EDITOR = 1
} AppMode;

typedef struct
{
  AppController *controller;
  AppMode current_mode;
} ModeManager;

ModeManager *mode_manager_new(AppController *controller);
void mode_manager_free(ModeManager *manager);

AppMode mode_manager_get_mode(const ModeManager *manager);
gboolean mode_manager_is_read_mode(const ModeManager *manager);
gboolean mode_manager_is_editor_mode(const ModeManager *manager);

void mode_manager_set_mode(ModeManager *manager, AppMode mode);
void mode_manager_enter_read_mode(ModeManager *manager);
void mode_manager_enter_editor_mode(ModeManager *manager);

#endif