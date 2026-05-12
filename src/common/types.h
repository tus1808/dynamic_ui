#ifndef TYPES_H
#define TYPES_H

#include <glib.h>

typedef struct {
    gint x;
    gint y;
} Point;

typedef struct {
    gint width;
    gint height;
} Size;

typedef struct {
    gchar *content;
    Point  location;
    Size   size;
    gint   font_size;
    gchar *font_style;
    gchar *color;
} MarqueeConfig;

typedef struct {
    gchar   *format;
    Point    location;
    Size     size;
    gint     font_size;
    gchar   *font_style;
    gchar   *color;
    gboolean visibility;
} ClockConfig;

typedef struct {
    gchar *ip_address;
    gint   ws_port;
} NetworkConfig;

typedef struct {
    gchar         *window_title;
    gchar         *background;
    gchar         *layout_file_path;
    gchar         *css_file_path;
    gchar         *editor_password;
    MarqueeConfig  marquee;
    ClockConfig    time_zone;
    ClockConfig    date_zone;
    NetworkConfig  network;
} AppConfig;

typedef struct {
    gchar   *_id;
    gint     index;
    Point    location;
    gint     width;
    gint     height;
    gchar   *value;
    gint     font_size;
    gchar   *font_style;
    gchar   *font_color;
    gchar   *background_color;
    gboolean background_transparent;
} LayoutItem;

#endif
