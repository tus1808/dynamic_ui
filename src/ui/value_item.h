#ifndef VALUE_ITEM_H
#define VALUE_ITEM_H

#include <gtk/gtk.h>
#include "common/types.h"

GtkWidget *ui_value_item_create(const LayoutItem *item);
void ui_value_item_set_value(GtkWidget *widget, const char *value);

#endif