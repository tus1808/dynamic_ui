#include "ui/canvas.h"

#include "ui/value_item.h"

static GPtrArray *ui_canvas_ensure_selected_items(GtkWidget *canvas) {
    GPtrArray *selected_items;

    if (!canvas)
        return NULL;

    selected_items = g_object_get_data(G_OBJECT(canvas), "selected-items");
    if (!selected_items) {
        selected_items = g_ptr_array_new();
        g_object_set_data(G_OBJECT(canvas), "selected-items", selected_items);
    }

    return selected_items;
}

GtkWidget *ui_canvas_create(void) {
    GtkWidget *canvas = gtk_fixed_new();

    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);

    gtk_widget_add_events(
        canvas,
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
    );

    g_object_set_data(G_OBJECT(canvas), "canvas-interactive", GINT_TO_POINTER(FALSE));

    return canvas;
}

void ui_canvas_render_items(GtkWidget *canvas, GPtrArray *items, GPtrArray *value_items) {
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

        if (value_items)
            g_ptr_array_add(value_items, widget);
    }
}

GtkWidget *ui_canvas_add_item(GtkWidget *canvas, LayoutItem *item) {
    GtkWidget *widget;

    if (!GTK_IS_FIXED(canvas) || !item)
        return NULL;

    widget = ui_value_item_create(item);
    if (!widget)
        return NULL;

    g_object_set_data(G_OBJECT(widget), "layout-item", item);

    gtk_fixed_put(GTK_FIXED(canvas), widget, item->x, item->y);
    gtk_widget_show(widget);

    return widget;
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

gboolean ui_canvas_is_item_selected(GtkWidget *canvas, GtkWidget *item_widget) {
    GPtrArray *selected_items;

    if (!canvas || !item_widget)
        return FALSE;

    selected_items = ui_canvas_ensure_selected_items(canvas);
    if (!selected_items)
        return FALSE;

    for (guint i = 0; i < selected_items->len; i++) {
        if (g_ptr_array_index(selected_items, i) == item_widget)
            return TRUE;
    }

    return FALSE;
}

void ui_canvas_clear_selection(GtkWidget *canvas) {
    GPtrArray *selected_items;

    if (!canvas)
        return;

    selected_items = ui_canvas_ensure_selected_items(canvas);
    if (!selected_items)
        return;

    for (guint i = 0; i < selected_items->len; i++) {
        GtkWidget *widget = g_ptr_array_index(selected_items, i);
        ui_value_item_set_selected(widget, FALSE);
    }

    g_ptr_array_set_size(selected_items, 0);
}

void ui_canvas_select_only(GtkWidget *canvas, GtkWidget *item_widget) {
    GPtrArray *selected_items;

    if (!canvas || !item_widget)
        return;

    selected_items = ui_canvas_ensure_selected_items(canvas);
    if (!selected_items)
        return;

    ui_canvas_clear_selection(canvas);
    ui_value_item_set_selected(item_widget, TRUE);
    g_ptr_array_add(selected_items, item_widget);
}

void ui_canvas_toggle_item_selection(GtkWidget *canvas, GtkWidget *item_widget) {
    GPtrArray *selected_items;

    if (!canvas || !item_widget)
        return;

    selected_items = ui_canvas_ensure_selected_items(canvas);
    if (!selected_items)
        return;

    for (guint i = 0; i < selected_items->len; i++) {
        if (g_ptr_array_index(selected_items, i) == item_widget) {
            ui_value_item_set_selected(item_widget, FALSE);
            g_ptr_array_remove_index(selected_items, i);
            return;
        }
    }

    ui_value_item_set_selected(item_widget, TRUE);
    g_ptr_array_add(selected_items, item_widget);
}

GPtrArray *ui_canvas_get_selected_items(GtkWidget *canvas) {
    if (!canvas)
        return NULL;

    return ui_canvas_ensure_selected_items(canvas);
}

void ui_canvas_remove_item(GtkWidget *canvas, GtkWidget *item_widget) {
    if (!canvas || !item_widget)
        return;

    if (ui_canvas_is_item_selected(canvas, item_widget))
        ui_canvas_toggle_item_selection(canvas, item_widget);

    gtk_container_remove(GTK_CONTAINER(canvas), item_widget);
}