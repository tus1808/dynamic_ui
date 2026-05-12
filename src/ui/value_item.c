#include "ui/value_item.h"

#include <gdk/gdk.h>
#include <pango/pango.h>

#include "ui/canvas.h"
#include "ui/resize_handle.h"

static gint compute_font_size(gint height) {
    gint size = (gint)(height * 0.40);
    if (size < 10) size = 10;
    if (size > 72) size = 72;
    return size;
}

static const gchar *font_weight_for_style(const gchar *style) {
    if (!style)
        return "bold";
    if (g_ascii_strcasecmp(style, "italic") == 0)
        return "normal";
    if (g_ascii_strcasecmp(style, "bold_italic") == 0)
        return "bold";
    if (g_ascii_strcasecmp(style, "normal") == 0)
        return "normal";
    /* default and explicit "bold" */
    return "bold";
}

static const gchar *font_slant_for_style(const gchar *style) {
    if (!style)
        return "normal";
    if (g_ascii_strcasecmp(style, "italic") == 0 ||
        g_ascii_strcasecmp(style, "bold_italic") == 0)
        return "italic";
    return "normal";
}

static gchar *generate_class_name(GtkWidget *widget) {
    return g_strdup_printf("vi-%p", (void *)widget);
}

static void apply_inline_css(GtkWidget *event_box, const LayoutItem *item) {
    GtkStyleContext *ctx;
    GtkCssProvider *provider;
    GtkWidget *label_value;
    gchar *class_name;
    gchar *css;
    gchar *bg_decl;
    gint applied_font_size;

    if (!event_box || !item)
        return;

    ctx = gtk_widget_get_style_context(event_box);
    class_name = generate_class_name(event_box);
    gtk_style_context_add_class(ctx, class_name);

    label_value = g_object_get_data(G_OBJECT(event_box), "value-label");

    applied_font_size = item->font_size > 0 ? item->font_size : compute_font_size(item->height);

    if (item->background_transparent) {
        bg_decl = g_strdup("background-color: transparent; background-image: none;");
    } else {
        bg_decl = g_strdup_printf(
            "background-color: %s;",
            item->background_color ? item->background_color : "#FFFFFF"
        );
    }

    css = g_strdup_printf(
        ".%s { %s }"
        ".%s label { "
        "  font-size: %dpx; "
        "  font-weight: %s; "
        "  font-style: %s; "
        "  color: %s; "
        "}",
        class_name, bg_decl,
        class_name,
        applied_font_size,
        font_weight_for_style(item->font_style),
        font_slant_for_style(item->font_style),
        item->font_color ? item->font_color : "#000000"
    );

    g_free(bg_decl);

    /* Per-widget provider — attached to both event-box and inner label contexts so the
     * cascade reaches the label. Detach any previous provider from BOTH contexts first. */
    provider = g_object_get_data(G_OBJECT(event_box), "css-provider");
    if (provider) {
        gtk_style_context_remove_provider(ctx, GTK_STYLE_PROVIDER(provider));
        if (label_value) {
            gtk_style_context_remove_provider(
                gtk_widget_get_style_context(label_value), GTK_STYLE_PROVIDER(provider)
            );
        }
        g_object_unref(provider);
    }

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider(ctx, GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    if (label_value) {
        gtk_style_context_add_provider(gtk_widget_get_style_context(label_value),
                                       GTK_STYLE_PROVIDER(provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    }

    g_object_set_data(G_OBJECT(event_box), "css-provider", provider);

    /* track the explicit font size so apply_font_size can know whether to override */
    g_object_set_data(G_OBJECT(event_box), "explicit-font-size",
                      GINT_TO_POINTER(item->font_size));
    g_object_set_data(G_OBJECT(event_box), "last-font-size",
                      GINT_TO_POINTER(applied_font_size));

    g_free(css);
    g_free(class_name);
}

void ui_value_item_apply_font_size(GtkWidget *event_box, gint height) {
    GtkWidget *label;
    PangoAttrList *attrs;
    gint explicit_font_size;
    gint font_size;
    gint last;

    label = g_object_get_data(G_OBJECT(event_box), "value-label");
    if (!label)
        return;

    /* When the item has an explicit font_size > 0, the CSS provider
     * is authoritative; skip the height-derived auto-resize. */
    explicit_font_size = GPOINTER_TO_INT(
        g_object_get_data(G_OBJECT(event_box), "explicit-font-size")
    );
    if (explicit_font_size > 0)
        return;

    font_size = compute_font_size(height);
    last = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(event_box), "last-font-size"));
    if (font_size == last)
        return;
    g_object_set_data(G_OBJECT(event_box), "last-font-size", GINT_TO_POINTER(font_size));

    attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_size_new_absolute(font_size * PANGO_SCALE));
    gtk_label_set_attributes(GTK_LABEL(label), attrs);
    pango_attr_list_unref(attrs);
}

void ui_value_item_refresh_style(GtkWidget *widget) {
    LayoutItem *item;

    if (!widget)
        return;
    item = ui_value_item_get_layout_item(widget);
    if (!item)
        return;

    apply_inline_css(widget, item);
    gtk_widget_queue_draw(widget);
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    GtkWidget *canvas;
    LayoutItem *item;
    guint resize_edges;
    GPtrArray *selected_items;
    (void)user_data;

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
                GINT_TO_POINTER(selected_item->location.x)
            );
            g_object_set_data(
                G_OBJECT(selected_widget),
                "origin-y",
                GINT_TO_POINTER(selected_item->location.y)
            );
        }
    }

    g_object_set_data(G_OBJECT(widget), "drag-start-root-x", GINT_TO_POINTER((int)event->x_root));
    g_object_set_data(G_OBJECT(widget), "drag-start-root-y", GINT_TO_POINTER((int)event->y_root));
    g_object_set_data(G_OBJECT(widget), "origin-x", GINT_TO_POINTER((int)item->location.x));
    g_object_set_data(G_OBJECT(widget), "origin-y", GINT_TO_POINTER((int)item->location.y));
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
    (void)user_data;

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

                selected_item->location.x = base_x + dx;
                selected_item->location.y = base_y + dy;

                gtk_fixed_move(
                    GTK_FIXED(canvas),
                    selected_widget,
                    selected_item->location.x,
                    selected_item->location.y
                );
            }

            return TRUE;
        }

        item->location.x = origin_x + dx;
        item->location.y = origin_y + dy;
        gtk_fixed_move(GTK_FIXED(canvas), widget, item->location.x, item->location.y);

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

        item->location.x = new_x;
        item->location.y = new_y;
        item->width = new_w;
        item->height = new_h;

        gtk_fixed_move(GTK_FIXED(canvas), widget, new_x, new_y);
        gtk_widget_set_size_request(widget, new_w, new_h);
        ui_value_item_apply_font_size(widget, new_h);

        gtk_widget_queue_resize(widget);
        gtk_widget_queue_draw(widget);
        gtk_widget_queue_resize(canvas);

        return TRUE;
    }

    return FALSE;
}

static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    (void)event;
    (void)user_data;
    g_object_set_data(G_OBJECT(widget), "dragging", GINT_TO_POINTER(0));
    g_object_set_data(G_OBJECT(widget), "resizing", GINT_TO_POINTER(0));
    g_object_set_data(G_OBJECT(widget), "resize-edges", GINT_TO_POINTER(RESIZE_NONE));

    ui_resize_reset_widget_cursor(widget);
    return TRUE;
}

static gboolean on_leave_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
    gboolean dragging = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "dragging"));
    gboolean resizing = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "resizing"));
    (void)event;
    (void)user_data;

    if (!dragging && !resizing)
        ui_resize_reset_widget_cursor(widget);

    return FALSE;
}

static void on_event_box_destroy(GtkWidget *widget, gpointer user_data) {
    GtkCssProvider *provider;
    (void)user_data;

    provider = g_object_get_data(G_OBJECT(widget), "css-provider");
    if (provider) {
        g_object_unref(provider);
        g_object_set_data(G_OBJECT(widget), "css-provider", NULL);
    }
}

GtkWidget *ui_value_item_create(const LayoutItem *item) {
    GtkWidget *event_box = NULL;
    GtkWidget *vbox = NULL;
    GtkWidget *label_value = NULL;
    GtkWidget *label_id = NULL;
    gchar *value_text = NULL;

    if (!item)
        return NULL;

    event_box = gtk_event_box_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(event_box), "event-box");
    gtk_widget_set_size_request(event_box, item->width, item->height);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(vbox, GTK_ALIGN_FILL);
    gtk_widget_set_valign(vbox, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(vbox, TRUE);
    gtk_widget_set_vexpand(vbox, TRUE);

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
    gtk_box_pack_start(GTK_BOX(vbox), label_value, TRUE, TRUE, 0);

    label_id = gtk_label_new(item->_id ? item->_id : "");
    gtk_style_context_add_class(gtk_widget_get_style_context(label_id), "id-label");
    gtk_widget_set_halign(label_id, GTK_ALIGN_CENTER);
    gtk_widget_set_no_show_all(label_id, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), label_id, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(event_box), vbox);

    gtk_widget_add_events(
        event_box,
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
            GDK_BUTTON1_MOTION_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
    );

    g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(event_box, "motion-notify-event", G_CALLBACK(on_motion), NULL);
    g_signal_connect(event_box, "button-release-event", G_CALLBACK(on_button_release), NULL);
    g_signal_connect(event_box, "leave-notify-event", G_CALLBACK(on_leave_notify), NULL);
    g_signal_connect(event_box, "destroy", G_CALLBACK(on_event_box_destroy), NULL);

    g_object_set_data(G_OBJECT(event_box), "value-label", label_value);
    g_object_set_data(G_OBJECT(event_box), "id-label", label_id);
    g_object_set_data(G_OBJECT(event_box), "layout-item", (gpointer)item);
    g_object_set_data(G_OBJECT(event_box), "selected", GINT_TO_POINTER(FALSE));
    g_object_set_data(G_OBJECT(event_box), "editable", GINT_TO_POINTER(FALSE));

    gtk_widget_show_all(event_box);

    apply_inline_css(event_box, item);
    if (item->font_size <= 0)
        ui_value_item_apply_font_size(event_box, item->height);

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

void ui_value_item_set_read_mode(GtkWidget *widget, gboolean read_mode) {
    GtkStyleContext *ctx;
    GtkWidget *label_id;
    LayoutItem *item;

    if (!widget)
        return;

    ctx = gtk_widget_get_style_context(widget);
    label_id = g_object_get_data(G_OBJECT(widget), "id-label");

    if (read_mode) {
        gtk_style_context_add_class(ctx, "read-mode");
        if (label_id)
            gtk_widget_hide(label_id);
    } else {
        gtk_style_context_remove_class(ctx, "read-mode");
        if (label_id) {
            item = ui_value_item_get_layout_item(widget);
            gtk_label_set_text(GTK_LABEL(label_id), item && item->_id ? item->_id : "");
            gtk_widget_show(label_id);
            gtk_widget_queue_resize(widget);
        }
    }
}
