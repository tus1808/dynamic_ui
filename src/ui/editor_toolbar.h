#ifndef EDITOR_TOOLBAR_H
#define EDITOR_TOOLBAR_H

#include <gtk/gtk.h>

typedef struct _AppController AppController;

typedef struct {
    AppController *controller;
    GtkWidget *container;
    GtkWidget *btn_add_one;
    GtkWidget *btn_add_many;
    GtkWidget *btn_delete;
    GtkWidget *btn_change_background;
} EditorToolbar;

EditorToolbar *editor_toolbar_new(AppController *controller);
void editor_toolbar_free(EditorToolbar *toolbar);

GtkWidget *editor_toolbar_get_widget(EditorToolbar *toolbar);

void editor_toolbar_show(EditorToolbar *toolbar);
void editor_toolbar_hide(EditorToolbar *toolbar);

#endif