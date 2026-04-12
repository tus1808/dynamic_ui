#ifndef CANVAS_H
#define CANVAS_H

#include <gtk/gtk.h>

#include "common/types.h"

GtkWidget *ui_canvas_create(void);
void ui_canvas_render_items(GtkWidget *canvas, GPtrArray *items, GPtrArray *value_items);
GtkWidget *ui_canvas_add_item(GtkWidget *canvas, LayoutItem *item);

void ui_canvas_set_interactive(GtkWidget *canvas, gboolean interactive);
gboolean ui_canvas_is_interactive(GtkWidget *canvas);

gboolean ui_canvas_is_item_selected(GtkWidget *canvas, GtkWidget *item_widget);
void ui_canvas_clear_selection(GtkWidget *canvas);

void ui_canvas_select_only(GtkWidget *canvas, GtkWidget *item_widget);
void ui_canvas_toggle_item_selection(GtkWidget *canvas, GtkWidget *item_widget);
GPtrArray *ui_canvas_get_selected_items(GtkWidget *canvas);

void ui_canvas_remove_item(GtkWidget *canvas, GtkWidget *item_widget);

#endif