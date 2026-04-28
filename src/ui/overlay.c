#include "ui/overlay.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <string.h>

#ifdef HAVE_GSTREAMER
#include <gst/gst.h>
#endif

#ifdef HAVE_GSTREAMER
static gboolean path_is_video(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext)
        return FALSE;
    return (g_ascii_strcasecmp(ext, ".mp4") == 0 ||
            g_ascii_strcasecmp(ext, ".mkv") == 0 ||
            g_ascii_strcasecmp(ext, ".avi") == 0);
}

static gboolean on_video_bus_message(GstBus *bus, GstMessage *msg, gpointer user_data) {
    GstElement *pipeline = user_data;
    (void)bus;

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
        gst_element_seek_simple(
            pipeline,
            GST_FORMAT_TIME,
            GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
            0
        );
    } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
        GError *err = NULL;
        gchar *dbg = NULL;
        gst_message_parse_error(msg, &err, &dbg);
        g_warning("[VIDEO] GStreamer error: %s (%s)", err ? err->message : "?", dbg ? dbg : "?");
        g_clear_error(&err);
        g_free(dbg);
    }

    return TRUE;
}

static void on_overlay_destroy(GtkWidget *overlay, gpointer user_data) {
    (void)user_data;
    ui_overlay_stop_video(overlay);
}
#endif

void ui_overlay_stop_video(GtkWidget *overlay) {
#ifdef HAVE_GSTREAMER
    GstElement *pipeline;
    GtkWidget *video_widget;

    if (!GTK_IS_OVERLAY(overlay))
        return;

    pipeline = g_object_get_data(G_OBJECT(overlay), "video-pipeline");
    video_widget = g_object_get_data(G_OBJECT(overlay), "video-sink-widget");

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        g_object_set_data(G_OBJECT(overlay), "video-pipeline", NULL);
    }

    if (video_widget && GTK_IS_WIDGET(video_widget)) {
        gtk_container_remove(GTK_CONTAINER(overlay), video_widget);
        g_object_set_data(G_OBJECT(overlay), "video-sink-widget", NULL);
    }
#else
    (void)overlay;
#endif
}

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

#ifdef HAVE_GSTREAMER
    g_signal_connect(overlay, "destroy", G_CALLBACK(on_overlay_destroy), NULL);
#endif

    return overlay;
}

gboolean ui_overlay_set_background(GtkWidget *overlay, const char *image_path) {
    GtkWidget *background;

    if (!GTK_IS_OVERLAY(overlay) || !image_path)
        return FALSE;

    ui_overlay_stop_video(overlay);

    background = g_object_get_data(G_OBJECT(overlay), "background-image");

#ifdef HAVE_GSTREAMER
    if (path_is_video(image_path)) {
        GstElement *pipeline = NULL;
        GstElement *sink_elem = NULL;
        GtkWidget *video_widget = NULL;
        GstBus *bus = NULL;
        gchar *quoted_path = NULL;
        gchar *pipeline_str = NULL;

        quoted_path = g_shell_quote(image_path);
        pipeline_str = g_strdup_printf(
            "filesrc location=%s ! decodebin ! videoconvert ! gtksink name=sink sync=true",
            quoted_path
        );
        g_free(quoted_path);

        pipeline = gst_parse_launch(pipeline_str, NULL);
        g_free(pipeline_str);

        if (!pipeline) {
            g_warning("[VIDEO] Failed to create GStreamer pipeline for: %s", image_path);
            return FALSE;
        }

        sink_elem = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
        if (!sink_elem) {
            g_warning("[VIDEO] Could not find gtksink element");
            gst_object_unref(pipeline);
            return FALSE;
        }

        g_object_get(sink_elem, "widget", &video_widget, NULL);
        gst_object_unref(sink_elem);

        if (!video_widget) {
            g_warning("[VIDEO] gtksink returned no widget");
            gst_object_unref(pipeline);
            return FALSE;
        }

        gtk_widget_set_halign(video_widget, GTK_ALIGN_FILL);
        gtk_widget_set_valign(video_widget, GTK_ALIGN_FILL);
        gtk_widget_set_hexpand(video_widget, TRUE);
        gtk_widget_set_vexpand(video_widget, TRUE);

        if (GTK_IS_WIDGET(background))
            gtk_widget_hide(background);

        gtk_overlay_add_overlay(GTK_OVERLAY(overlay), video_widget);
        gtk_overlay_reorder_overlay(GTK_OVERLAY(overlay), video_widget, 0);
        gtk_overlay_set_overlay_pass_through(GTK_OVERLAY(overlay), video_widget, TRUE);
        gtk_widget_show(video_widget);

        g_object_set_data_full(
            G_OBJECT(overlay), "video-pipeline",
            gst_object_ref(pipeline), (GDestroyNotify)gst_object_unref
        );
        g_object_set_data(G_OBJECT(overlay), "video-sink-widget", video_widget);

        bus = gst_element_get_bus(pipeline);
        gst_bus_add_watch(bus, on_video_bus_message, pipeline);
        gst_object_unref(bus);

        gst_element_set_state(
            (GstElement *)g_object_get_data(G_OBJECT(overlay), "video-pipeline"),
            GST_STATE_PLAYING
        );

        gst_object_unref(pipeline);
        return TRUE;
    }
#endif

    if (!GTK_IS_IMAGE(background))
        return FALSE;

    gtk_widget_show(background);

    GError *error = NULL;
    GdkPixbufAnimation *animation = gdk_pixbuf_animation_new_from_file(image_path, &error);
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
