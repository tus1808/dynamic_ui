#ifndef RESIZE_HANDLE_H
#define RESIZE_HANDLE_H

#include <gtk/gtk.h>

#define RESIZE_BORDER 8
#define MIN_ITEM_WIDTH 60
#define MIN_ITEM_HEIGHT 30

typedef enum
{
  RESIZE_NONE = 0,
  RESIZE_LEFT = 1 << 0,
  RESIZE_RIGHT = 1 << 1,
  RESIZE_TOP = 1 << 2,
  RESIZE_BOTTOM = 1 << 3
} ResizeEdge;

guint ui_resize_detect_resize_edges(GtkWidget *widget, gdouble x, gdouble y);
GdkCursorType ui_resize_cursor_type_for_edges(guint edges);
void ui_resize_set_widget_cursor(GtkWidget *widget, GdkCursorType cursor_type);
void ui_resize_reset_widget_cursor(GtkWidget *widget);

#endif