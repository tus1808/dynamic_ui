#include "input/auth_manager.h"

#include "app/controller.h"
#include "app/state.h"
#include "mode/manager.h"

typedef struct {
    AuthManager *manager;
    GtkWidget *dialog;
    GtkWidget *entry;
    GtkWidget *error_label;
} AuthDialogData;

static gchar *compute_hash(const gchar *password) {
    if (!password)
        return NULL;

    return g_compute_checksum_for_string(G_CHECKSUM_SHA256, password, -1);
}

gboolean auth_manager_check_password(AuthManager *manager, const gchar *password) {
    const gchar *expected_hash;
    gchar *actual_hash;
    gboolean ok;

    if (!manager || !manager->controller || !manager->controller->state)
        return FALSE;

    expected_hash = manager->controller->state->config.editor_password;

    if (!expected_hash || expected_hash[0] == '\0')
        return TRUE;

    if (!password)
        return FALSE;

    actual_hash = compute_hash(password);
    if (!actual_hash)
        return FALSE;

    ok = (g_strcmp0(expected_hash, actual_hash) == 0);

    g_free(actual_hash);
    return ok;
}

static void auth_dialog_submit(AuthDialogData *data) {
    const gchar *text;

    if (!data || !data->manager || !data->entry)
        return;

    text = gtk_entry_get_text(GTK_ENTRY(data->entry));

    if (auth_manager_check_password(data->manager, text)) {
        ModeManager *mode_manager = data->manager->controller->mode_manager;
        gtk_widget_destroy(data->dialog);
        mode_manager_enter_editor_mode(mode_manager);
        return;
    }

    gtk_label_set_text(GTK_LABEL(data->error_label), "Wrong password!!!");
    gtk_label_set_xalign(GTK_LABEL(data->error_label), 0.0f);
    gtk_entry_set_text(GTK_ENTRY(data->entry), "");
    gtk_widget_grab_focus(data->entry);
}

static void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    AuthDialogData *data = user_data;

    if (!data)
        return;

    if (response_id == GTK_RESPONSE_OK) {
        auth_dialog_submit(data);

        return;
    }

    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void on_dialog_destroy(GtkWidget *widget, gpointer user_data) {
    (void)widget;
    g_free(user_data);
}

AuthManager *auth_manager_new(AppController *controller) {
    AuthManager *manager = g_new0(AuthManager, 1);
    manager->controller = controller;

    return manager;
}

void auth_manager_free(AuthManager *manager) {
    if (!manager)
        return;
    g_free(manager);
}

void auth_manager_request_editor_access(AuthManager *manager) {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *error_label;
    AuthDialogData *data;

    if (!manager || !manager->controller || !manager->controller->state)
        return;
    if (!manager->controller->state->window)
        return;

    dialog = gtk_dialog_new_with_buttons(
        "Editor Authentication",
        GTK_WINDOW(manager->controller->state->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Cancel",
        GTK_RESPONSE_CANCEL,
        "_OK",
        GTK_RESPONSE_OK,
        NULL
    );

    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    label = gtk_label_new("Enter password:");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);

    entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);

    error_label = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(error_label), 0.0f);

    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), error_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    data = g_new0(AuthDialogData, 1);
    data->manager = manager;
    data->dialog = dialog;
    data->entry = entry;
    data->error_label = error_label;

    g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), data);
    g_signal_connect(dialog, "destroy", G_CALLBACK(on_dialog_destroy), data);

    gtk_widget_set_name(box, "auth-box");
    gtk_widget_set_name(dialog, "auth-dialog");
    gtk_widget_set_name(entry, "auth-entry");
    gtk_widget_set_name(error_label, "auth-error-label");

    gtk_widget_show_all(dialog);
    gtk_widget_grab_focus(entry);
}