#ifndef LOADER_H
#define LOADER_H

#include <glib.h>
#include "common/types.h"

gboolean config_loader_load_app_config(const char *file_path, AppConfig *app_config);
gboolean config_loader_save_app_config(const char *file_path, const AppConfig *app_config);
void config_loader_free_app_config(AppConfig *config);

#endif