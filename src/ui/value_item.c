#include "ui/value_item.h"

GtkWidget *ui_value_item_create(const LayoutItem *item)
{
  GtkWidget *event_box = NULL;
  GtkWidget *box = NULL;
  GtkWidget *label_value = NULL;
  gchar *value_text = NULL;

  if (!item)
    return NULL;

  event_box = gtk_event_box_new();
  gtk_style_context_add_class(
      gtk_widget_get_style_context(event_box),
      "value-item");
  gtk_widget_set_size_request(event_box, item->width, item->height);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_container_add(GTK_CONTAINER(event_box), box);

  value_text = g_strdup_printf("%s", item->value ? item->value : "--");
  label_value = gtk_label_new(value_text);
  gtk_widget_set_halign(label_value, GTK_ALIGN_START);
  g_free(value_text);

  gtk_box_pack_start(GTK_BOX(box), label_value, FALSE, FALSE, 0);

  g_object_set_data(G_OBJECT(event_box), "value-label", label_value);

  gtk_widget_show_all(event_box);
  return event_box;
}

void ui_value_item_set_value(GtkWidget *widget, const char *value)
{
  GtkWidget *label_value = NULL;

  if (!widget)
    return;

  label_value = g_object_get_data(G_OBJECT(widget), "value-label");
  if (!label_value)
    return;

  gtk_label_set_text(GTK_LABEL(label_value), value ? value : "--");
}