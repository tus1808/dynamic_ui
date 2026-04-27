#include "ui/marquee.h"

#include <gtk/gtk.h>
#include <pango/pangocairo.h>

#define MARQUEE_HEIGHT  40
#define MARQUEE_SPEED   1.5   /* pixels per tick at ~60fps */
#define MARQUEE_FONT    "Sans Bold 16"

typedef struct {
    gchar   *content;
    gdouble  x_offset;
    gint     text_width;
    gboolean initialized;
    guint    timer_id;
} MarqueeState;

static void marquee_state_free(gpointer data) {
    MarqueeState *state = data;

    if (!state)
        return;

    g_free(state->content);
    g_free(state);
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    MarqueeState *state = user_data;
    GtkAllocation alloc;
    PangoLayout *layout;
    PangoFontDescription *font_desc;
    int tw, th;

    if (!state || !state->content || state->content[0] == '\0')
        return TRUE;

    gtk_widget_get_allocation(widget, &alloc);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.65);
    cairo_paint(cr);

    layout = pango_cairo_create_layout(cr);
    font_desc = pango_font_description_from_string(MARQUEE_FONT);
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);
    pango_layout_set_text(layout, state->content, -1);
    pango_layout_get_pixel_size(layout, &tw, &th);

    state->text_width = tw;

    if (!state->initialized) {
        state->x_offset = (gdouble)alloc.width;
        state->initialized = TRUE;
    }

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_move_to(cr, state->x_offset, (alloc.height - th) / 2.0);
    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
    return TRUE;
}

static gboolean on_tick(gpointer user_data) {
    GtkWidget *widget = user_data;
    MarqueeState *state;
    GtkAllocation alloc;

    if (!GTK_IS_WIDGET(widget))
        return G_SOURCE_REMOVE;

    state = g_object_get_data(G_OBJECT(widget), "marquee-state");
    if (!state)
        return G_SOURCE_REMOVE;

    gtk_widget_get_allocation(widget, &alloc);

    state->x_offset -= MARQUEE_SPEED;

    /* wrap around: when text has fully scrolled off the left edge */
    if (state->x_offset + state->text_width < 0)
        state->x_offset = (gdouble)alloc.width;

    gtk_widget_queue_draw(widget);
    return G_SOURCE_CONTINUE;
}

static void on_destroy(GtkWidget *widget, gpointer user_data) {
    MarqueeState *state = user_data;

    if (state->timer_id) {
        g_source_remove(state->timer_id);
        state->timer_id = 0;
    }
}

GtkWidget *ui_marquee_create(const gchar *content) {
    GtkWidget *widget;
    MarqueeState *state;

    state = g_new0(MarqueeState, 1);
    state->content = g_strdup(content ? content : "");
    state->initialized = FALSE;

    widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(widget, -1, MARQUEE_HEIGHT);
    gtk_widget_set_halign(widget, GTK_ALIGN_FILL);
    gtk_widget_set_valign(widget, GTK_ALIGN_END);
    gtk_widget_set_hexpand(widget, TRUE);

    g_object_set_data_full(G_OBJECT(widget), "marquee-state", state, marquee_state_free);

    g_signal_connect(widget, "draw", G_CALLBACK(on_draw), state);
    g_signal_connect(widget, "destroy", G_CALLBACK(on_destroy), state);

    state->timer_id = g_timeout_add(16, on_tick, widget);

    return widget;
}
