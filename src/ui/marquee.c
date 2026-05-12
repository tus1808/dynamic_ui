#include "ui/marquee.h"

#include <gtk/gtk.h>
#include <pango/pangocairo.h>

#include "common/color.h"

#define DEFAULT_MARQUEE_HEIGHT  40
#define MARQUEE_SPEED           1.5   /* pixels per tick at ~60fps */

typedef struct {
    gchar   *content;
    gchar   *font_style;
    gint     font_size;
    double   r, g, b, a;
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
    g_free(state->font_style);
    g_free(state);
}

static gchar *build_font_desc(const gchar *style, gint size) {
    /* maps "bold", "italic", "bold_italic", "normal" -> Pango font description */
    const gchar *pango_style = "";
    if (style) {
        if (g_ascii_strcasecmp(style, "bold") == 0)
            pango_style = "Bold";
        else if (g_ascii_strcasecmp(style, "italic") == 0)
            pango_style = "Italic";
        else if (g_ascii_strcasecmp(style, "bold_italic") == 0)
            pango_style = "Bold Italic";
    }
    return g_strdup_printf("Sans %s %d", pango_style, size > 0 ? size : 16);
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    MarqueeState *state = user_data;
    GtkAllocation alloc;
    PangoLayout *layout;
    PangoFontDescription *font_desc;
    gchar *font_str;
    int tw, th;

    if (!state || !state->content || state->content[0] == '\0')
        return TRUE;

    gtk_widget_get_allocation(widget, &alloc);

    /* semi-transparent black background behind the text */
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.65);
    cairo_paint(cr);

    layout = pango_cairo_create_layout(cr);
    font_str = build_font_desc(state->font_style, state->font_size);
    font_desc = pango_font_description_from_string(font_str);
    g_free(font_str);
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);
    pango_layout_set_text(layout, state->content, -1);
    pango_layout_get_pixel_size(layout, &tw, &th);

    state->text_width = tw;

    if (!state->initialized) {
        state->x_offset = (gdouble)alloc.width;
        state->initialized = TRUE;
    }

    cairo_set_source_rgba(cr, state->r, state->g, state->b, state->a);
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

    if (state->x_offset + state->text_width < 0)
        state->x_offset = (gdouble)alloc.width;

    gtk_widget_queue_draw(widget);
    return G_SOURCE_CONTINUE;
}

static void on_destroy(GtkWidget *widget, gpointer user_data) {
    MarqueeState *state = user_data;
    (void)widget;

    if (state->timer_id) {
        g_source_remove(state->timer_id);
        state->timer_id = 0;
    }
}

GtkWidget *ui_marquee_create(const MarqueeConfig *cfg) {
    GtkWidget *widget;
    MarqueeState *state;
    gint width, height;

    if (!cfg)
        return NULL;

    state = g_new0(MarqueeState, 1);
    state->content     = g_strdup(cfg->content ? cfg->content : "");
    state->font_style  = g_strdup(cfg->font_style ? cfg->font_style : "bold");
    state->font_size   = cfg->font_size > 0 ? cfg->font_size : 16;
    state->initialized = FALSE;

    /* default text colour: white */
    if (!color_parse_hex(cfg->color, &state->r, &state->g, &state->b, &state->a)) {
        state->r = state->g = state->b = 1.0;
        state->a = 1.0;
    }

    width  = cfg->size.width;
    height = cfg->size.height > 0 ? cfg->size.height : DEFAULT_MARQUEE_HEIGHT;

    widget = gtk_drawing_area_new();
    /* width == 0 means: caller will stretch (e.g. set hexpand) — request -1 */
    gtk_widget_set_size_request(widget, width > 0 ? width : -1, height);
    gtk_widget_set_halign(widget, width > 0 ? GTK_ALIGN_START : GTK_ALIGN_FILL);
    gtk_widget_set_valign(widget, GTK_ALIGN_START);
    if (width == 0)
        gtk_widget_set_hexpand(widget, TRUE);

    g_object_set_data_full(G_OBJECT(widget), "marquee-state", state, marquee_state_free);

    g_signal_connect(widget, "draw", G_CALLBACK(on_draw), state);
    g_signal_connect(widget, "destroy", G_CALLBACK(on_destroy), state);

    state->timer_id = g_timeout_add(16, on_tick, widget);

    return widget;
}
