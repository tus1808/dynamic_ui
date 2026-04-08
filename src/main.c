#include <gtk/gtk.h>
#include "app/controller.h"

static void on_activate(GtkApplication *app, gpointer user_data)
{
  AppController *controller = user_data;
  app_controller_activate(controller, app);
}

int main(int argc, char **argv)
{
  AppController *controller = app_controller_new();

  GtkApplication *app = gtk_application_new("com.example.dynamic_ui", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), controller);

  int status = g_application_run(G_APPLICATION(app), argc, argv);

  g_object_unref(app);
  app_controller_free(controller);
  return status;
}