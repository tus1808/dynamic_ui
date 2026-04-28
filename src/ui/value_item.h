#ifndef VALUE_ITEM_H
#define VALUE_ITEM_H

#include <gtk/gtk.h>
#include "common/types.h"

GtkWidget *ui_value_item_create(const LayoutItem *item);
void ui_value_item_set_value(GtkWidget *widget, const char *value);

void ui_value_item_set_selected(GtkWidget *widget, gboolean selected);
gboolean ui_value_item_is_selected(GtkWidget *widget);

void ui_value_item_set_editable(GtkWidget *widget, gboolean editable);
gboolean ui_value_item_is_editable(GtkWidget *widget);

void ui_value_item_apply_font_size(GtkWidget *widget, gint height);
void ui_value_item_set_read_mode(GtkWidget *widget, gboolean read_mode);

LayoutItem *ui_value_item_get_layout_item(GtkWidget *widget);

#endif