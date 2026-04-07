#ifndef CANVAS_H
#define CANVAS_H

#include <gtk/gtk.h>

GtkWidget *ui_canvas_create(void);
gboolean ui_canvas_set_background(GtkWidget *canvas, const char *image_path);

#endif