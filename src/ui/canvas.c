#include "ui/canvas.h"

GtkWidget *ui_canvas_create(void) {
    GtkWidget *canvas = gtk_fixed_new();
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);
    g_object_set_data(G_OBJECT(canvas), "canvas-interactive", GINT_TO_POINTER(FALSE));

    return canvas;
}

gboolean ui_canvas_set_background(GtkWidget *canvas, const char *image_path) {
    GError *error = NULL;
    GdkPixbuf *pixbuf = NULL;
    GtkWidget *image = NULL;
    GtkWidget *old_image = NULL;

    if (!GTK_IS_FIXED(canvas) || !image_path)
        return FALSE;

    pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
    if (!pixbuf) {
        g_warning(
            "Cannot load background '%s': %s",
            image_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);

        return FALSE;
    }
    image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    if (!GTK_IS_IMAGE(image))
        return FALSE;

    old_image = g_object_get_data(G_OBJECT(canvas), "background-image");
    if (old_image)
        gtk_container_remove(GTK_CONTAINER(canvas), old_image);

    gtk_fixed_put(GTK_FIXED(canvas), image, 0, 0);
    gtk_widget_show(image);

    g_object_set_data(G_OBJECT(canvas), "background-image", image);

    return TRUE;
}

void ui_canvas_render_items(GtkWidget *canvas, GPtrArray *items) {
    if (!GTK_IS_FIXED(canvas) || !items)
        return;

    for (guint i = 0; i < items->len; i++) {
        LayoutItem *item = g_ptr_array_index(items, i);
        GtkWidget *widget;

        if (!item)
            continue;

        widget = ui_value_item_create(item);
        if (!widget)
            continue;

        g_object_set_data(G_OBJECT(widget), "layout-item", item);
        gtk_fixed_put(GTK_FIXED(canvas), widget, item->x, item->y);
        gtk_widget_show(widget);
    }
}

void ui_canvas_set_interactive(GtkWidget *canvas, gboolean interactive) {
    if (!canvas)
        return;

    g_object_set_data(G_OBJECT(canvas), "canvas-interactive", GINT_TO_POINTER(interactive ? 1 : 0));
}

gboolean ui_canvas_is_interactive(GtkWidget *canvas) {
    if (!canvas)
        return FALSE;

    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(canvas), "canvas-interactive")) != 0;
}

void ui_canvas_set_selected_item(GtkWidget *canvas, GtkWidget *item_widget) {
    GtkWidget *old_item;

    if (!canvas)
        return;

    old_item = g_object_get_data(G_OBJECT(canvas), "selected-item");
    if (old_item && old_item != item_widget)
        ui_value_item_set_selected(old_item, FALSE);

    g_object_set_data(G_OBJECT(canvas), "selected-item", item_widget);

    if (item_widget)
        ui_value_item_set_selected(item_widget, TRUE);
}

GtkWidget *ui_canvas_get_selected_item(GtkWidget *canvas) {
    if (!canvas)
        return NULL;

    return g_object_get_data(G_OBJECT(canvas), "selected-item");
}

void ui_canvas_clear_selection(GtkWidget *canvas) { ui_canvas_set_selected_item(canvas, NULL); }

GtkWidget *ui_canvas_add_item(GtkWidget *canvas, LayoutItem *item) {
    GtkWidget *widget;

    if (!GTK_IS_FIXED(canvas) || !item)
        return NULL;

    widget = ui_value_item_create(item);
    if (!widget)
        return NULL;

    g_object_set_data(G_OBJECT(widget), "layout_item", item);

    gtk_fixed_put(GTK_FIXED(canvas), widget, item->x, item->y);
    gtk_widget_show(widget);

    return widget;
}