#ifndef CANVAS_H
#define CANVAS_H

#include <gtk/gtk.h>
#include "common/types.h"

#include "ui/value_item.h"

GtkWidget *ui_canvas_create(void);
gboolean ui_canvas_set_background(GtkWidget *canvas, const char *image_path);
void ui_canvas_render_items(GtkWidget *canvas, GPtrArray *items);

void ui_canvas_set_interactive(GtkWidget *canvas, gboolean interactive);
gboolean ui_canvas_is_interactive(GtkWidget *canvas);

void ui_canvas_set_selected_item(GtkWidget *canvas, GtkWidget *item_widget);
GtkWidget *ui_canvas_get_selected_item(GtkWidget *canvas);
void ui_canvas_clear_selection(GtkWidget *canvas);

GtkWidget *ui_canvas_add_item(GtkWidget *canvas, LayoutItem *item);

#endif