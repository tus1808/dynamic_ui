#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <gtk/gtk.h>

typedef struct _AppController AppController;

typedef struct {
    AppController *controller;
} AuthManager;

AuthManager *auth_manager_new(AppController *controller);
void auth_manager_free(AuthManager *manager);

gboolean auth_manager_check_password(AuthManager *manager, const gchar *password);
void auth_manager_request_editor_access(AuthManager *manager);

#endif