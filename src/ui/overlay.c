#include "ui/overlay.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

GtkWidget *ui_overlay_create(GtkWidget *canvas) {
    GtkWidget *overlay;
    GtkWidget *background;

    if (!GTK_IS_FIXED(canvas))
        return NULL;

    overlay = gtk_overlay_new();
    gtk_widget_set_hexpand(overlay, TRUE);
    gtk_widget_set_vexpand(overlay, TRUE);

    background = gtk_image_new();
    gtk_widget_set_halign(background, GTK_ALIGN_FILL);
    gtk_widget_set_valign(background, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(background, TRUE);
    gtk_widget_set_vexpand(background, TRUE);
    gtk_container_add(GTK_CONTAINER(overlay), background);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), canvas);
    gtk_widget_set_halign(canvas, GTK_ALIGN_FILL);
    gtk_widget_set_valign(canvas, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);

    g_object_set_data(G_OBJECT(overlay), "overlay-canvas", canvas);
    g_object_set_data(G_OBJECT(overlay), "background-image", background);
    g_object_set_data(G_OBJECT(overlay), "overlay-toolbar", NULL);

    return overlay;
}

gboolean ui_overlay_set_background(GtkWidget *overlay, const char *image_path) {
    GError *error = NULL;
    GdkPixbufAnimation *animation = NULL;
    GtkWidget *background;

    if (!GTK_IS_OVERLAY(overlay) || !image_path)
        return FALSE;

    background = g_object_get_data(G_OBJECT(overlay), "background-image");
    if (!GTK_IS_IMAGE(background))
        return FALSE;

    animation = gdk_pixbuf_animation_new_from_file(image_path, &error);
    if (!animation) {
        g_warning(
            "Cannot load background '%s': %s",
            image_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);
        return FALSE;
    }

    gtk_image_set_from_animation(GTK_IMAGE(background), animation);
    g_object_unref(animation);

    return TRUE;
}

GtkWidget *ui_overlay_get_canvas(GtkWidget *overlay) {
    if (!GTK_IS_OVERLAY(overlay))
        return NULL;

    return g_object_get_data(G_OBJECT(overlay), "overlay-canvas");
}

GtkWidget *ui_overlay_get_background(GtkWidget *overlay) {
    if (!GTK_IS_OVERLAY(overlay))
        return NULL;

    return g_object_get_data(G_OBJECT(overlay), "background-image");
}

GtkWidget *ui_overlay_get_toolbar(GtkWidget *overlay) {
    if (!GTK_IS_OVERLAY(overlay))
        return NULL;

    return g_object_get_data(G_OBJECT(overlay), "overlay-toolbar");
}

void ui_overlay_set_marquee(GtkWidget *overlay, GtkWidget *marquee) {
    GtkWidget *old;

    if (!GTK_IS_OVERLAY(overlay) || !GTK_IS_WIDGET(marquee))
        return;

    old = g_object_get_data(G_OBJECT(overlay), "overlay-marquee");
    if (old)
        gtk_container_remove(GTK_CONTAINER(overlay), old);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), marquee);
    gtk_widget_show(marquee);

    g_object_set_data(G_OBJECT(overlay), "overlay-marquee", marquee);
}

void ui_overlay_set_toolbar(GtkWidget *overlay, GtkWidget *toolbar) {
    GtkWidget *old_toolbar;

    if (!GTK_IS_OVERLAY(overlay) || !GTK_IS_WIDGET(toolbar))
        return;

    old_toolbar = g_object_get_data(G_OBJECT(overlay), "overlay-toolbar");
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