#include "ui/clock_widget.h"

#include <gtk/gtk.h>
#include <pango/pangocairo.h>
#include <string.h>

#include "common/color.h"

#define DEFAULT_CLOCK_FONT_SIZE 14

typedef struct {
    gchar  *format;       /* user-provided, e.g. "hh:mm:ss" */
    gchar  *strftime_fmt; /* translated, e.g. "%H:%M:%S" */
    gchar  *font_style;
    gint    font_size;
    double  r, g, b, a;
    gint    width;        /* 0 = auto-fit to text */
    gint    height;       /* 0 = auto-fit to text */
    guint   timer_id;
} ClockState;

static void clock_state_free(gpointer data) {
    ClockState *s = data;
    if (!s)
        return;
    g_free(s->format);
    g_free(s->strftime_fmt);
    g_free(s->font_style);
    g_free(s);
}

/* Translate user tokens to strftime tokens.
 * hh -> %H, ss -> %S, dd -> %d, yyyy -> %Y, yy -> %y.
 * mm meaning depends on is_date: minutes when FALSE, months when TRUE.
 * Any character not part of a known token is copied verbatim. */
static gchar *translate_format(const char *src, gboolean is_date) {
    GString *out;
    const char *p;

    out = g_string_new(NULL);
    if (!src) {
        g_string_append(out, is_date ? "%d/%m/%y" : "%H:%M:%S");
        return g_string_free(out, FALSE);
    }

    p = src;
    while (*p) {
        if (strncmp(p, "yyyy", 4) == 0) {
            g_string_append(out, "%Y");
            p += 4;
        } else if (strncmp(p, "yy", 2) == 0) {
            g_string_append(out, "%y");
            p += 2;
        } else if (strncmp(p, "hh", 2) == 0) {
            g_string_append(out, "%H");
            p += 2;
        } else if (strncmp(p, "mm", 2) == 0) {
            g_string_append(out, is_date ? "%m" : "%M");
            p += 2;
        } else if (strncmp(p, "ss", 2) == 0) {
            g_string_append(out, "%S");
            p += 2;
        } else if (strncmp(p, "dd", 2) == 0) {
            g_string_append(out, "%d");
            p += 2;
        } else {
            g_string_append_c(out, *p);
            p++;
        }
    }

    return g_string_free(out, FALSE);
}

static gchar *build_font_desc(const gchar *style, gint size) {
    const gchar *pango_style = "";
    if (style) {
        if (g_ascii_strcasecmp(style, "bold") == 0)
            pango_style = "Bold";
        else if (g_ascii_strcasecmp(style, "italic") == 0)
            pango_style = "Italic";
        else if (g_ascii_strcasecmp(style, "bold_italic") == 0)
            pango_style = "Bold Italic";
    }
    return g_strdup_printf("Sans %s %d", pango_style,
                           size > 0 ? size : DEFAULT_CLOCK_FONT_SIZE);
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    ClockState *s = user_data;
    GDateTime *now;
    gchar *text;
    PangoLayout *layout;
    PangoFontDescription *font_desc;
    gchar *font_str;
    int tw, th;
    GtkAllocation alloc;

    if (!s)
        return TRUE;

    gtk_widget_get_allocation(widget, &alloc);

    now = g_date_time_new_now_local();
    text = g_date_time_format(now, s->strftime_fmt);
    g_date_time_unref(now);
    if (!text)
        return TRUE;

    layout = pango_cairo_create_layout(cr);
    font_str = build_font_desc(s->font_style, s->font_size);
    font_desc = pango_font_description_from_string(font_str);
    g_free(font_str);
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);
    pango_layout_set_text(layout, text, -1);
    pango_layout_get_pixel_size(layout, &tw, &th);

    /* re-size widget to fit text if auto-sizing */
    if (s->width == 0 || s->height == 0) {
        gint new_w = s->width  > 0 ? s->width  : tw + 8;
        gint new_h = s->height > 0 ? s->height : th + 4;
        if (new_w != alloc.width || new_h != alloc.height) {
            gtk_widget_set_size_request(widget, new_w, new_h);
        }
    }

    cairo_set_source_rgba(cr, s->r, s->g, s->b, s->a);
    cairo_move_to(cr, 4.0, (alloc.height - th) / 2.0);
    pango_cairo_show_layout(cr, layout);

    g_free(text);
    g_object_unref(layout);
    return TRUE;
}

static gboolean on_tick(gpointer user_data) {
    GtkWidget *widget = user_data;

    if (!GTK_IS_WIDGET(widget))
        return G_SOURCE_REMOVE;

    gtk_widget_queue_draw(widget);
    return G_SOURCE_CONTINUE;
}

static void on_destroy(GtkWidget *widget, gpointer user_data) {
    ClockState *s = user_data;
    (void)widget;

    if (s && s->timer_id) {
        g_source_remove(s->timer_id);
        s->timer_id = 0;
    }
}

GtkWidget *ui_clock_create(const ClockConfig *cfg, gboolean is_date) {
    GtkWidget *widget;
    ClockState *s;

    if (!cfg || !cfg->visibility)
        return NULL;

    s = g_new0(ClockState, 1);
    s->format       = g_strdup(cfg->format ? cfg->format : (is_date ? "dd/mm/yy" : "hh:mm:ss"));
    s->strftime_fmt = translate_format(s->format, is_date);
    s->font_style   = g_strdup(cfg->font_style ? cfg->font_style : "bold");
    s->font_size    = cfg->font_size > 0 ? cfg->font_size : DEFAULT_CLOCK_FONT_SIZE;
    s->width        = cfg->size.width;
    s->height       = cfg->size.height;

    if (!color_parse_hex(cfg->color, &s->r, &s->g, &s->b, &s->a)) {
        s->r = s->g = s->b = 0.0;
        s->a = 1.0;
    }

    widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(widget,
                                s->width  > 0 ? s->width  : 100,
                                s->height > 0 ? s->height : 30);

    g_object_set_data_full(G_OBJECT(widget), "clock-state", s, clock_state_free);

    g_signal_connect(widget, "draw", G_CALLBACK(on_draw), s);
    g_signal_connect(widget, "destroy", G_CALLBACK(on_destroy), s);

    s->timer_id = g_timeout_add_seconds(1, on_tick, widget);

    return widget;
}
