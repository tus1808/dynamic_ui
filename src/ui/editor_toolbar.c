#include "ui/editor_toolbar.h"

#include "app/controller.h"

static void on_add_one_clicked(GtkButton *button, gpointer user_data) {
    EditorToolbar *toolbar = user_data;
    (void)button;

    if (!toolbar || !toolbar->controller)
        return;
    g_print("[EDITOR] Add one item\n");
    /*TODO:
        editor_mode_add_one_item(toolbar->controller->editor_mode);
    */
}

static void on_add_many_clicked(GtkButton *button, gpointer user_data) {
    EditorToolbar *toolbar = user_data;
    (void)button;

    if (!toolbar || !toolbar->controller)
        return;

    g_print("[EDITOR] Add many items\n");
    /*TODO:
        editor_mode_add_many_item(toolbar->controller->editor_mode);
    */
}

static void on_delete_clicked(GtkButton *button, gpointer user_data) {
    EditorToolbar *toolbar = user_data;
    (void)button;

    if (!toolbar || !toolbar->controller)
        return;

    g_print("[EDITOR] Delete selected item\n");

    /*TODO:
        editor_mode_delete_selected_item(toolbar->controller->editor_mode);
    */
}

static void on_change_background_clicked(GtkButton *button, gpointer user_data) {
    EditorToolbar *toolbar = user_data;
    (void)button;

    if (!toolbar || !toolbar->controller)
        return;

    g_print("[EDITOR] Change Background");
    /*TODO:
        editor_mode_add_change_background(toolbar->controller->editor_mode);
    */
}

EditorToolbar *editor_toolbar_new(AppController *controller) {
    EditorToolbar *toolbar;
    GtkWidget *box;

    toolbar = g_new0(EditorToolbar, 1);
    toolbar->controller = controller;

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    toolbar->container = box;

    gtk_widget_set_name(box, "editor-toolbar");
    gtk_widget_set_halign(box, GTK_ALIGN_END);
    gtk_widget_set_valign(box, GTK_ALIGN_START);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_end(box, 12);

    toolbar->btn_add_one = gtk_button_new_with_label("+");
    toolbar->btn_add_many = gtk_button_new_with_label("++");
    toolbar->btn_delete = gtk_button_new_with_label("x");
    toolbar->btn_change_background = gtk_button_new_with_label("BG");

    gtk_widget_set_name(toolbar->btn_add_one, "editor-toolbar-btn");
    gtk_widget_set_name(toolbar->btn_add_many, "editor-toolbar-btn");
    gtk_widget_set_name(toolbar->btn_delete, "editor-toolbar-btn");
    gtk_widget_set_name(toolbar->btn_change_background, "editor-toolbar-btn");

    gtk_box_pack_start(GTK_BOX(box), toolbar->btn_add_one, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), toolbar->btn_add_many, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), toolbar->btn_delete, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), toolbar->btn_change_background, FALSE, FALSE, 0);

    g_signal_connect(toolbar->btn_add_one, "clicked", G_CALLBACK(on_add_one_clicked), toolbar);
    g_signal_connect(toolbar->btn_add_many, "clicked", G_CALLBACK(on_add_many_clicked), toolbar);
    g_signal_connect(toolbar->btn_delete, "clicked", G_CALLBACK(on_delete_clicked), toolbar);
    g_signal_connect(
        toolbar->btn_change_background,
        "clicked",
        G_CALLBACK(on_change_background_clicked),
        toolbar
    );

    return toolbar;
}

void editor_toolbar_free(EditorToolbar *toolbar) {
    if (!toolbar)
        return;

    g_free(toolbar);
}

GtkWidget *editor_toolbar_get_widget(EditorToolbar *toolbar) {
    if (!toolbar)
        return NULL;

    return toolbar->container;
}

void editor_toolbar_show(EditorToolbar *toolbar) {
    if (!toolbar || !toolbar->container)
        return;

    g_print("[TOOLBAR] show\n");
    gtk_widget_show_all(toolbar->container);
}

void editor_toolbar_hide(EditorToolbar *toolbar) {
    if (!toolbar || !toolbar->container)
        return;

    g_print("[TOOLBAR] hide\n");
    gtk_widget_hide(toolbar->container);
}