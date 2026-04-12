#include "ui/overlay.h"

GtkWidget *ui_overlay_create(GtkWidget *canvas) {
    GtkWidget *overlay;

    if (!GTK_IS_FIXED(canvas))
        return NULL;

    overlay = gtk_overlay_new();
    gtk_widget_set_hexpand(overlay, TRUE);
    gtk_widget_set_vexpand(overlay, TRUE);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), canvas);
    gtk_widget_set_halign(canvas, GTK_ALIGN_FILL);
    gtk_widget_set_valign(canvas, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);

    g_object_set_data(G_OBJECT(overlay), "overlay-canvas", canvas);

    return overlay;
}

gboolean ui_overlay_set_background(GtkWidget *overlay, const char *image_path) {
    GError *error = NULL;
    GdkPixbuf *pixbuf = NULL;
    GtkWidget *image = NULL;
    GtkWidget *old_image = NULL;

    if (!GTK_IS_OVERLAY(overlay) || !image_path)
        return FALSE;

    pixbuf = gdk_pixbuf_new_from_file(image_path, &error);
    if (!pixbuf) {
        g_warning(
            "Cannot load background '%s': %s",
            image_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);

        return FALSE;
    }

    image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    old_image = g_object_get_data(G_OBJECT(overlay), "background-image");
    if (old_image)
        gtk_container_remove(GTK_CONTAINER(overlay), old_image);

    gtk_container_add(GTK_CONTAINER(overlay), image);
    gtk_widget_show(image);
    gtk_widget_set_sensitive(image, FALSE);
    g_object_set_data(G_OBJECT(overlay), "background-image", image);

    return TRUE;
}

GtkWidget *ui_overlay_get_canvas(GtkWidget *overlay) {
    if (!GTK_IS_OVERLAY(overlay))
        return NULL;

    return g_object_get_data(G_OBJECT(overlay), "overlay_canvas");
}

GtkWidget *ui_overlay_get_background(GtkWidget *overlay) {
    if (!GTK_IS_OVERLAY(overlay))
        return NULL;

    return g_object_get_data(G_OBJECT(overlay), "background-image");
}

GtkWidget *ui_overlay_get_toolbar(GtkWidget *overlay) {
    if (!GTK_IS_OVERLAY(overlay))
        return NULL;

    return g_object_get_data(G_OBJECT(overlay), "overlay_toolbar");
}

void ui_overlay_set_toolbar(GtkWidget *overlay, GtkWidget *toolbar) {
    GtkWidget *old_toolbar;

    if (!GTK_IS_OVERLAY(overlay) || !GTK_IS_WIDGET(toolbar))
        return;

    old_toolbar = g_object_get_data(G_OBJECT(overlay), "overlay_toolbar");
    if (old_toolbar)
        gtk_container_remove(GTK_CONTAINER(overlay), old_toolbar);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), toolbar);

    gtk_widget_set_halign(toolbar, GTK_ALIGN_END);
    gtk_widget_set_valign(toolbar, GTK_ALIGN_START);
    gtk_widget_set_margin_top(toolbar, 12);
    gtk_widget_set_margin_end(toolbar, 12);

    gtk_widget_show(toolbar);

    g_object_set_data(G_OBJECT(overlay), "overlay-toolbar", toolbar);
}