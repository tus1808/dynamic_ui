#ifndef TYPES_H
#define TYPES_H

#include <glib.h>

typedef struct {
    gchar *window_title;
    gchar *background;
    gchar *layout_file_path;
    gchar *css_file_path;
    gchar *editor_password;
    gchar *marquee_content;
} AppConfig;

typedef struct {
    gchar *_id;
    gint x;
    gint y;
    gint width;
    gint height;
    gchar *value;
} LayoutItem;

#endif