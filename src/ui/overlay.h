#ifndef OVERLAY_H
#define OVERLAY_H

#include <gtk/gtk.h>

GtkWidget *ui_overlay_create(GtkWidget *canvas);
gboolean ui_overlay_set_background(GtkWidget *overlay, const char *image_path);
GtkWidget *ui_overlay_get_canvas(GtkWidget *overlay);
void ui_overlay_set_toolbar(GtkWidget *overlay, GtkWidget *toolbar);
GtkWidget *ui_overlay_get_background(GtkWidget *overlay);

#endif