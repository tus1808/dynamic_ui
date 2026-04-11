#include "ui/resize_handle.h"

guint ui_resize_detect_resize_edges(GtkWidget *widget, gdouble x, gdouble y) {
    GtkAllocation alloc;
    guint edges = RESIZE_NONE;

    gtk_widget_get_allocation(widget, &alloc);

    if (x <= RESIZE_BORDER)
        edges |= RESIZE_LEFT;
    if (x >= alloc.width - RESIZE_BORDER)
        edges |= RESIZE_RIGHT;
    if (y <= RESIZE_BORDER)
        edges |= RESIZE_TOP;
    if (y >= alloc.height - RESIZE_BORDER)
        edges |= RESIZE_BOTTOM;

    return edges;
}

GdkCursorType ui_resize_cursor_type_for_edges(guint edges) {
    switch (edges) {
    case RESIZE_LEFT:
        return GDK_LEFT_SIDE;
    case RESIZE_RIGHT:
        return GDK_RIGHT_SIDE;
    case RESIZE_TOP:
        return GDK_TOP_SIDE;
    case RESIZE_BOTTOM:
        return GDK_BOTTOM_SIDE;
    case RESIZE_TOP | RESIZE_LEFT:
        return GDK_TOP_LEFT_CORNER;
    case RESIZE_TOP | RESIZE_RIGHT:
        return GDK_TOP_RIGHT_CORNER;
    case RESIZE_BOTTOM | RESIZE_LEFT:
        return GDK_BOTTOM_LEFT_CORNER;
    case RESIZE_BOTTOM | RESIZE_RIGHT:
        return GDK_BOTTOM_RIGHT_CORNER;
    default:
        return GDK_LEFT_PTR;
    }
}

void ui_resize_set_widget_cursor(GtkWidget *widget, GdkCursorType cursor_type) {
    GdkWindow *window;
    GdkDisplay *display;
    GdkCursor *cursor;

    if (!gtk_widget_get_realized(widget))
        return;

    window = gtk_widget_get_window(widget);
    if (!window)
        return;

    display = gdk_window_get_display(window);
    cursor = gdk_cursor_new_for_display(display, cursor_type);
    gdk_window_set_cursor(window, cursor);
    g_object_unref(cursor);
}

void ui_resize_reset_widget_cursor(GtkWidget *widget) {
    ui_resize_set_widget_cursor(widget, GDK_LEFT_PTR);
}