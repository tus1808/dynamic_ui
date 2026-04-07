#include "ui/window.h"

GtkWidget *ui_window_create(GtkApplication *app, GtkWidget *child)
{
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Dynamic UI");
  gtk_window_fullscreen(GTK_WINDOW(window));

  if (child)
  {
    gtk_container_add(GTK_CONTAINER(window), child);
  }

  return window;
}