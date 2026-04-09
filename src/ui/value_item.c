#include "ui/value_item.h"

#include <gdk/gdk.h>

#include "ui/canvas.h"

static gboolean is_resize_zone(GtkWidget *widget, gdouble x, gdouble y)
{
  GtkAllocation alloc;
  gtk_widget_get_allocation(widget, &alloc);

  return x >= alloc.width - 12 && y >= alloc.height - 12;
}

static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer *user_data)
{
  GtkWidget *canvas;
  LayoutItem *item;

  if (event->button != 1)
    return FALSE;

  canvas = gtk_widget_get_parent(widget);
  item = ui_value_item_get_layout_item(widget);
  if (!canvas || !item)
    return FALSE;
  if (!ui_canvas_is_interactive(canvas))
    return FALSE;

  ui_canvas_set_selected_item(canvas, widget);
  g_object_set_data(G_OBJECT(widget), "drag-start-x", GINT_TO_POINTER((int)event->x));
  g_object_set_data(G_OBJECT(widget), "drag-start-y", GINT_TO_POINTER((int)event->y));
  g_object_set_data(G_OBJECT(widget), "origin-x", GINT_TO_POINTER((int)item->x));
  g_object_set_data(G_OBJECT(widget), "origin-y", GINT_TO_POINTER((int)item->y));
  g_object_set_data(G_OBJECT(widget), "origin-width", GINT_TO_POINTER((int)item->width));
  g_object_set_data(G_OBJECT(widget), "origin-height", GINT_TO_POINTER((int)item->height));

  if (is_resize_zone(widget, event->x, event->y))
  {
    g_object_set_data(G_OBJECT(widget), "resizing", GINT_TO_POINTER(1));
    g_object_set_data(G_OBJECT(widget), "dragging", GINT_TO_POINTER(0));
  }
  else
  {
    g_object_set_data(G_OBJECT(widget), "resizing", GINT_TO_POINTER(0));
    g_object_set_data(G_OBJECT(widget), "dragging", GINT_TO_POINTER(1));
  }

  return TRUE;
}

static gboolean on_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
  GtkWidget *canvas;
  LayoutItem *item;

  gboolean dragging;
  gboolean resizing;

  int start_x, start_y;
  int origin_x, origin_y;
  int origin_w, origin_h;

  int dx, dy;

  canvas = gtk_widget_get_parent(widget);
  item = ui_value_item_get_layout_item(widget);

  if (!canvas || !item)
    return FALSE;
  if (!ui_canvas_is_interactive(canvas))
    return FALSE;

  dragging = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "dragging"));
  resizing = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "resizing"));

  if (!dragging && !resizing)
    return FALSE;

  start_x = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "drag-start-x"));
  start_y = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "drag-start-y"));

  origin_x = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-x"));
  origin_y = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-y"));

  origin_w = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-width"));
  origin_h = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "origin-height"));

  dx = (int)event->x - start_x;
  dy = (int)event->y - start_y;

  if (dragging)
  {
    item->x = origin_x + dx;
    item->y = origin_y + dy;

    gtk_fixed_move(GTK_FIXED(canvas), widget, item->x, item->y);
  }

  if (resizing)
  {
    int new_w = origin_w + dx;
    int new_h = origin_h + dy;

    if (new_w < 60)
      new_w = 60;
    if (new_h < 30)
      new_h = 30;

    item->width = new_w;
    item->height = new_h;

    gtk_widget_set_size_request(widget, new_w, new_h);
  }

  return TRUE;
}

static gboolean on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_date)
{
  g_object_set_data(G_OBJECT(widget), "dragging", GINT_TO_POINTER(0));
  g_object_set_data(G_OBJECT(widget), "resizing", GINT_TO_POINTER(0));

  return TRUE;
}

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

  // Event
  gtk_widget_add_events(event_box, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

  g_signal_connect(event_box, "button-press-event", G_CALLBACK(on_button_press), NULL);
  g_signal_connect(event_box, "motion_notify_event", G_CALLBACK(on_motion), NULL);
  g_signal_connect(event_box, "button-release-event", G_CALLBACK(on_button_release), NULL);

  g_object_set_data(G_OBJECT(event_box), "layout-item", (gpointer)item);
  g_object_set_data(G_OBJECT(event_box), "selected", GINT_TO_POINTER(FALSE));
  g_object_set_data(G_OBJECT(event_box), "editable", GINT_TO_POINTER(FALSE));

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

void ui_value_item_set_selected(GtkWidget *widget, gboolean selected)
{
  if (!widget)
    return;

  g_object_set_data(G_OBJECT(widget), "selected", GINT_TO_POINTER(selected ? 1 : 0));

  if (selected)
  {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "selected");
  }
  else
  {
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "selected");
  }
}

gboolean ui_value_item_is_selected(GtkWidget *widget)
{
  if (!widget)
    return FALSE;

  return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "selected")) != 0;
}

void ui_value_item_set_editable(GtkWidget *widget, gboolean editable)
{
  if (!widget)
    return;

  g_object_set_data(G_OBJECT(widget), "editable", GINT_TO_POINTER(editable ? 1 : 0));

  if (editable)
  {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), "selected");
  }
  else
  {
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), "selected");
  }
}

gboolean ui_value_item_is_editable(GtkWidget *widget)
{
  if (!widget)
    return FALSE;

  return GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "editable")) != 0;
}

LayoutItem *ui_value_item_get_layout_item(GtkWidget *widget)
{
  if (!widget)
    return NULL;

  return g_object_get_data(G_OBJECT(widget), "layout-item");
}