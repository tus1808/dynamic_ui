#include "ui/canvas.h"
#include "common/types.h"

GtkWidget *ui_canvas_create(void)
{
  GtkWidget *fixed = gtk_fixed_new();
  gtk_widget_set_hexpand(fixed, TRUE);
  gtk_widget_set_vexpand(fixed, TRUE);

  return fixed;
}

gboolean ui_canvas_set_background(GtkWidget *canvas, const char *image_path)
{
  GError *error = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkWidget *image = NULL;
  GtkWidget *old_image = NULL;

  if (!GTK_IS_FIXED(canvas) || !image_path)
    return FALSE;

  pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
  if (!pixbuf)
  {
    g_warning("Cannot load background '%s': %s", image_path, error ? error->message : "unknown error");
    g_clear_error(&error);

    return FALSE;
  }
  image = gtk_image_new_from_pixbuf(pixbuf);
  g_object_unref(pixbuf);

  if (!GTK_IS_IMAGE(image))
    return FALSE;

  old_image = g_object_get_data(G_OBJECT(canvas), "background-image");
  if (old_image)
    gtk_container_remove(GTK_CONTAINER(canvas), old_image);

  gtk_fixed_put(GTK_FIXED(canvas), image, 0, 0);
  gtk_widget_show(image);

  g_object_set_data(G_OBJECT(canvas), "background-image", image);

  return TRUE;
}

void ui_canvas_render_items(GtkWidget *canvas, GPtrArray *items)
{
  if (!GTK_IS_FIXED(canvas) || !items)
    return;

  for (guint i = 0; i < items->len; i++)
  {
    LayoutItem *item = g_ptr_array_index(items, i);
    if (!item)
      continue;

    GtkWidget *label = gtk_label_new(item->value ? item->value : "");
    gtk_widget_set_size_request(label, item->width, item->height);

    gtk_widget_set_name(label, "value-item");
    gtk_fixed_put(GTK_FIXED(canvas), label, item->x, item->y);
    gtk_widget_show(label);
  }
}