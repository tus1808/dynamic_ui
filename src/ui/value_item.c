#include "ui/value_item.h"

#include <gdk/gdk.h>

#include "ui/canvas.h"
#include "ui/resize_handle.h"

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GtkWidget *canvas;
    LayoutItem *item;
    guint resize_edges;
    GPtrArray *selected_items;

    if (event->button != 1)
        return FALSE;

    canvas = gtk_widget_get_parent(widget);
    item = ui_value_item_get_layout_item(widget);

    if (!canvas || !item)
        return FALSE;

    if (!ui_canvas_is_interactive(canvas))
        return FALSE;

    if (event->state & GDK_CONTROL_MASK) {
        ui_canvas_toggle_item_selection(canvas, widget);
    } else {
        if (!ui_canvas_is_item_selected(canvas, widget))
            ui_canvas_select_only(canvas, widget);
    }

    selected_items = ui_canvas_get_selected_items(canvas);

    if (selected_items && ui_canvas_is_item_selected(canvas, widget)) {
        for (guint i = 0; i < selected_items->len; i++) {
            GtkWidget *selected_widget = g_ptr_array_index(selected_items, i);
            LayoutItem *selected_item = ui_value_item_get_layout_item(selected_widget);

            if (!selected_item)
                continue;

            g_object_set_data(
                G_OBJECT(selected_widget),
                "origin-x",
                GINT_TO_POINTER(selected_item->x)
            );
            g_object_set_data(
                G_OBJECT(selected_widget),
                "origin-y",
                GINT_TO_POINTER(selected_item->y)
            );
        }
    }

    g_object_set_data(G_OBJECT(widget), "drag-start-root-x", GINT_TO_POINTER((int)event->x_root));
    g_object_set_data(G_OBJECT(widget), "drag-start-root-y", GINT_TO_POINTER((int)event->y_root));
    g_object_set_data(G_OBJECT(widget), "origin-x", GINT_TO_POINTER((int)item->x));
    g_object_set_data(G_OBJECT(widget), "origin-y", GINT_TO_POINTER((int)item->y));
    g_object_set_data(G_OBJECT(widget), "origin-width", GINT_TO_POINTER((int)item->width));
    g_object_set_data(G_OBJECT(widget), "origin-height", GINT_TO_POINTER((int)item->height));

    resize_edges = ui_resize_detect_resize_edges(widget, event->x, event->y);
    g_object_set_data(G_OBJECT(widget), "resize-edges", GINT_TO_POINTER(resize_edges));

    if (resize_edges != RESIZE_NONE) {
        g_object_set_data(G_OBJECT(widget), "resizing", GINT_TO_POINTER(1));
        g_object_set_data(G_OBJECT(widget), "dragging", GINT_TO_POINTER(0));
    } else {
        g_object_set_data(G_OBJECT(widget), "resizing", GINT_TO_POINTER(0));
        g_object_set_data(G_OBJECT(widget), "dragging", GINT_TO_POINTER(1));
    }

    return TRUE;
}

static gboolean on_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    GtkWidget *canvas;
    LayoutItem *item;
    gboolean dragging;
    gboolean resizing;
    int start_root_x, start_root_y;
    int origin_x, origin_y;
    int origin_w, origin_h;
    int dx, dy;

    canvas = gtk_widget_get_parent(widget);
    item = ui_value_item_get_layout_item(widget);

    if (!canvas || !item)
        return FALSE;
    if (!ui_canvas_is_interactive(canvas))
        return FALSE;

    dragging = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "dragging"));
    resizing = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "resizing"));

    /* Hover */
    if (!dragging && !resizing) {
        guint hover_edges = ui_resize_detect_resize_edges(widget, event->x, event->y);
        ui_resize_set_widget_cursor(widget, ui_resize_cursor_type_for_edges(hover_edges));
        return FALSE;
    }

    start_root_x = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "drag-start-root-x"));
    start_root_y = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "drag-start-root-y"));
    origin_x = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-x"));
    origin_y = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-y"));
    origin_w = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-width"));
    origin_h = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-height"));

    dx = ((int)event->x_root) - start_root_x;
    dy = ((int)event->y_root) - start_root_y;

    if (dragging) {
        GPtrArray *selected_items = ui_canvas_get_selected_items(canvas);

        if (selected_items && selected_items->len > 1 &&
            ui_canvas_is_item_selected(canvas, widget)) {
            for (guint i = 0; i < selected_items->len; i++) {
                GtkWidget *selected_widget = g_ptr_array_index(selected_items, i);
                LayoutItem *selected_item = ui_value_item_get_layout_item(selected_widget);

                if (!selected_item)
                    continue;

                int base_x =
                    GPOINTER_TO_INT(g_object_get_data(G_OBJECT(selected_widget), "origin-x"));
                int base_y =
                    GPOINTER_TO_INT(g_object_get_data(G_OBJECT(selected_widget), "origin-y"));

                selected_item->x = base_x + dx;
                selected_item->y = base_y + dy;

                gtk_fixed_move(
                    GTK_FIXED(canvas),
                    selected_widget,
                    selected_item->x,
                    selected_item->y
                );
            }

            return TRUE;
        }

        item->x = origin_x + dx;
        item->y = origin_y + dy;
        gtk_fixed_move(GTK_FIXED(canvas), widget, item->x, item->y);

        return TRUE;
    }

    if (resizing) {
        guint edges = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "resize-edges"));
        int new_x = origin_x;
        int new_y = origin_y;
        int new_w = origin_w;
        int new_h = origin_h;

        if (edges & RESIZE_LEFT) {
            new_x = origin_x + dx;
            new_w = origin_w - dx;
            if (new_w < MIN_ITEM_WIDTH) {
                new_w = MIN_ITEM_WIDTH;
                new_x = origin_x + (origin_w - MIN_ITEM_WIDTH);
            }
        }

        if (edges & RESIZE_RIGHT) {
            new_w = origin_w + dx;
            if (new_w < MIN_ITEM_WIDTH)
                new_w = MIN_ITEM_WIDTH;
        }

        if (edges & RESIZE_TOP) {
            new_y = origin_y + dy;
            new_h = origin_h - dy;
            if (new_h < MIN_ITEM_HEIGHT) {
                new_h = MIN_ITEM_HEIGHT;
                new_y = origin_y + (origin_h - MIN_ITEM_HEIGHT);
            }
        }

        if (edges & RESIZE_BOTTOM) {
            new_h = origin_h + dy;
            if (new_h < MIN_ITEM_HEIGHT)
                new_h = MIN_ITEM_HEIGHT;
        }

        item->x = new_x;
        item->y = new_y;
        item->width = new_w;
        item->height = new_h;

        gtk_fixed_move(GTK_FIXED(canvas), widget, new_x, new_y);
        gtk_widget_set_size_request(widget, new_w, new_h);

        gtk_widget_queue_resize(widget);
        gtk_widget_queue_draw(widget);
        gtk_widget_queue_resize(canvas);

        return TRUE;
    }

    return FALSE;
}

static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    g_object_set_data(G_OBJECT(widget), "dragging", GINT_TO_POINTER(0));
    g_object_set_data(G_OBJECT(widget), "resizing", GINT_TO_POINTER(0));
    g_object_set_data(G_OBJECT(widget), "resize-edges", GINT_TO_POINTER(RESIZE_NONE));

    ui_resize_reset_widget_cursor(widget);
    return TRUE;
}

static gboolean on_leave_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
    gboolean dragging = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "dragging"));
    gboolean resizing = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "resizing"));

    if (!dragging && !resizing)
        ui_resize_reset_widget_cursor(widget);

    return FALSE;
}

GtkWidget *ui_value_item_create(const LayoutItem *item) {
    GtkWidget *event_box = NULL;
    GtkWidget *label_value = NULL;
    gchar *value_text = NULL;

    if (!item)
        return NULL;

    event_box = gtk_event_box_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(event_box), "event-box");
    gtk_widget_set_size_request(event_box, item->width, item->height);

    value_text = g_strdup_printf("%s", item->value ? item->value : "--");
    label_value = gtk_label_new(value_text);
    g_free(value_text);

    gtk_label_set_xalign(GTK_LABEL(label_value), 0.5f);
    gtk_label_set_yalign(GTK_LABEL(label_value), 0.5f);
    gtk_label_set_justify(GTK_LABEL(label_value), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(label_value), TRUE);

    gtk_widget_set_halign(label_value, GTK_ALIGN_FILL);
    gtk_widget_set_valign(label_value, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(label_value, TRUE);
    gtk_widget_set_vexpand(label_value, TRUE);

    gtk_container_add(GTK_CONTAINER(event_box), label_value);
    g_object_set_data(G_OBJECT(event_box), "value-label", label_value);

    // Event
    gtk_widget_add_events(
        event_box,
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
            GDK_BUTTON1_MOTION_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
    );

    g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(event_box, "motion-notify-event", G_CALLBACK(on_motion), NULL);
    g_signal_connect(event_box, "button-release-event", G_CALLBACK(on_button_release), NULL);
    g_signal_connect(event_box, "leave-notify-event", G_CALLBACK(on_leave_notify), NULL);

    g_object_set_data(G_OBJECT(event_box), "value-label", label_value);
    g_object_set_data(G_OBJECT(event_box), "layout-item", (gpointer)item);
    g_object_set_data(G_OBJECT(event_box), "selected", GINT_TO_POINTER(FALSE));
    g_object_set_data(G_OBJECT(event_box), "editable", GINT_TO_POINTER(FALSE));

    gtk_widget_show_all(event_box);
    return event_box;
}

void ui_value_item_set_value(GtkWidget *widget, const char *value) {
    GtkWidget *label_value = NULL;

    if (!widget)
        return;

    label_value = g_object_get_data(G_OBJECT(widget), "value-label");
    if (!label_value)
        return;

    gtk_label_set_text(GTK_LABEL(label_value), value ? value : "--");
}

void ui_value_item_set_selected(GtkWidget *widget, gboolean selected) {
    GtkStyleContext *context;

    if (!widget)
        return;

    context = gtk_widget_get_style_context(widget);
    g_object_set_data(G_OBJECT(widget), "selected", GINT_TO_POINTER(selected ? 1 : 0));

    if (selected) {
        gtk_style_context_add_class(context, "selected");
    } else
        gtk_style_context_remove_class(context, "selected");
}

gboolean ui_value_item_is_selected(GtkWidget *widget) {
    if (!widget)
        return FALSE;

    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "selected")) != 0;
}

void ui_value_item_set_editable(GtkWidget *widget, gboolean editable) {
    GtkStyleContext *context;

    if (!widget)
        return;

    context = gtk_widget_get_style_context(widget);
    g_object_set_data(G_OBJECT(widget), "editable", GINT_TO_POINTER(editable ? 1 : 0));

    if (editable) {
        gtk_style_context_add_class(context, "editable");
    } else {
        gtk_style_context_remove_class(context, "editable");
    }
}

gboolean ui_value_item_is_editable(GtkWidget *widget) {
    if (!widget)
        return FALSE;

    return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "editable")) != 0;
}

LayoutItem *ui_value_item_get_layout_item(GtkWidget *widget) {
    if (!widget)
        return NULL;

    return g_object_get_data(G_OBJECT(widget), "layout-item");
}