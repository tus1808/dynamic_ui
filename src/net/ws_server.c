#include "net/ws_server.h"

#include <glib.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <string.h>

#include "common/time_setter.h"
#include "ui/value_item.h"

static SoupServer *g_server = NULL;

static GtkWidget *find_widget_by_id(AppState *state, const gchar *id) {
    if (!state || !state->value_items || !id)
        return NULL;

    for (guint i = 0; i < state->value_items->len; i++) {
        GtkWidget *w = g_ptr_array_index(state->value_items, i);
        LayoutItem *li = ui_value_item_get_layout_item(w);
        if (li && li->_id && strcmp(li->_id, id) == 0)
            return w;
    }
    return NULL;
}

static void dispatch_data(AppState *state, JsonObject *root) {
    JsonArray *values;
    guint i, n;

    if (!json_object_has_member(root, "values"))
        return;
    values = json_object_get_array_member(root, "values");
    if (!values)
        return;

    n = json_array_get_length(values);
    for (i = 0; i < n; i++) {
        JsonObject *entry = json_array_get_object_element(values, i);
        const gchar *id;
        const gchar *value;
        GtkWidget *w;

        if (!entry)
            continue;
        if (!json_object_has_member(entry, "_id") ||
            !json_object_has_member(entry, "value"))
            continue;

        id    = json_object_get_string_member(entry, "_id");
        value = json_object_get_string_member(entry, "value");

        w = find_widget_by_id(state, id);
        if (!w) {
            g_print("[WS] unknown item _id: '%s'\n", id ? id : "(null)");
            continue;
        }
        ui_value_item_set_value(w, value);
    }
}

static void dispatch_time(JsonObject *root) {
    if (!json_object_has_member(root, "year") ||
        !json_object_has_member(root, "month") ||
        !json_object_has_member(root, "day") ||
        !json_object_has_member(root, "hour") ||
        !json_object_has_member(root, "minute") ||
        !json_object_has_member(root, "second")) {
        g_warning("[WS] time payload missing required fields");
        return;
    }

    time_setter_apply(
        (int)json_object_get_int_member(root, "year"),
        (int)json_object_get_int_member(root, "month"),
        (int)json_object_get_int_member(root, "day"),
        (int)json_object_get_int_member(root, "hour"),
        (int)json_object_get_int_member(root, "minute"),
        (int)json_object_get_int_member(root, "second")
    );
}

static void on_ws_message(SoupWebsocketConnection *conn,
                          SoupWebsocketDataType    type,
                          GBytes                  *message,
                          gpointer                 user_data) {
    AppState *state = user_data;
    JsonParser *parser;
    GError *error = NULL;
    JsonNode *root_node;
    JsonObject *root_obj;
    const gchar *msg_type;
    gsize sz = 0;
    const gchar *data;
    (void)conn;

    if (type != SOUP_WEBSOCKET_DATA_TEXT)
        return;

    data = (const gchar *)g_bytes_get_data(message, &sz);
    if (!data || sz == 0)
        return;

    parser = json_parser_new();
    if (!json_parser_load_from_data(parser, data, (gssize)sz, &error)) {
        g_warning("[WS] bad JSON: %s", error ? error->message : "unknown");
        g_clear_error(&error);
        g_object_unref(parser);
        return;
    }

    root_node = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_OBJECT(root_node)) {
        g_warning("[WS] payload root must be an object");
        g_object_unref(parser);
        return;
    }

    root_obj = json_node_get_object(root_node);
    if (!json_object_has_member(root_obj, "type")) {
        g_warning("[WS] payload missing 'type'");
        g_object_unref(parser);
        return;
    }

    msg_type = json_object_get_string_member(root_obj, "type");
    if (g_strcmp0(msg_type, "data") == 0) {
        dispatch_data(state, root_obj);
    } else if (g_strcmp0(msg_type, "time") == 0) {
        dispatch_time(root_obj);
    } else {
        g_print("[WS] ignored unknown type: '%s'\n", msg_type ? msg_type : "(null)");
    }

    g_object_unref(parser);
}

static void on_ws_closed(SoupWebsocketConnection *conn, gpointer user_data) {
    (void)conn;
    (void)user_data;
    g_print("[WS] client disconnected\n");
}

static void on_websocket(SoupServer              *server,
                         SoupWebsocketConnection *conn,
                         const char              *path,
                         SoupClientContext       *client,
                         gpointer                 user_data) {
    AppState *state = user_data;
    (void)server;
    (void)path;
    (void)client;

    g_print("[WS] client connected from %s\n",
            soup_client_context_get_host(client));

    g_signal_connect(conn, "message", G_CALLBACK(on_ws_message), state);
    g_signal_connect(conn, "closed",  G_CALLBACK(on_ws_closed),  state);

    /* libsoup owns the connection; keep a ref so it isn't dropped */
    g_object_ref(conn);
    g_signal_connect_swapped(conn, "closed", G_CALLBACK(g_object_unref), conn);
}

gboolean ws_server_start(AppState *state, gint port) {
    GError *error = NULL;

    if (g_server) {
        g_warning("[WS] server already running");
        return FALSE;
    }
    if (!state) {
        g_warning("[WS] no AppState; cannot start");
        return FALSE;
    }
    if (port <= 0)
        port = 8765;

    g_server = soup_server_new(SOUP_SERVER_SERVER_HEADER, "dynamic_ui-ws", NULL);
    if (!g_server) {
        g_warning("[WS] failed to create SoupServer");
        return FALSE;
    }

    soup_server_add_websocket_handler(
        g_server,
        "/",     /* path */
        NULL,    /* origin */
        NULL,    /* protocols */
        on_websocket,
        state,
        NULL
    );

    if (!soup_server_listen_all(g_server, (guint)port, 0, &error)) {
        g_warning("[WS] could not listen on port %d: %s",
                  port, error ? error->message : "unknown error");
        g_clear_error(&error);
        g_object_unref(g_server);
        g_server = NULL;
        return FALSE;
    }

    g_print("[WS] listening on ws://0.0.0.0:%d/\n", port);
    return TRUE;
}

void ws_server_stop(void) {
    if (!g_server)
        return;
    soup_server_disconnect(g_server);
    g_object_unref(g_server);
    g_server = NULL;
}
