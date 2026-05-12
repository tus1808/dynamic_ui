#include "loader.h"
#include <json-glib/json-glib.h>

#define DEFAULT_FONT_STYLE       "bold"
#define DEFAULT_COLOR            "#000000"
#define DEFAULT_MARQUEE_FONT_SIZE 16
#define DEFAULT_MARQUEE_HEIGHT    40
#define DEFAULT_CLOCK_FONT_SIZE   14
#define DEFAULT_WS_PORT           8765

static gchar *json_dup_string_member(JsonObject *obj, const gchar *key) {
    if (!json_object_has_member(obj, key))
        return NULL;

    return g_strdup(json_object_get_string_member(obj, key));
}

static gchar *json_dup_string_or_default(JsonObject *obj, const gchar *key, const gchar *fallback) {
    gchar *out = json_dup_string_member(obj, key);
    if (!out)
        out = g_strdup(fallback);
    return out;
}

static gint json_get_int_or(JsonObject *obj, const gchar *key, gint fallback) {
    if (!json_object_has_member(obj, key))
        return fallback;
    return (gint)json_object_get_int_member(obj, key);
}

static gboolean json_get_bool_or(JsonObject *obj, const gchar *key, gboolean fallback) {
    if (!json_object_has_member(obj, key))
        return fallback;
    return json_object_get_boolean_member(obj, key);
}

static void json_get_point(JsonObject *parent, const gchar *key, Point *out) {
    JsonObject *sub;

    out->x = 0;
    out->y = 0;

    if (!json_object_has_member(parent, key))
        return;
    sub = json_object_get_object_member(parent, key);
    if (!sub)
        return;

    out->x = json_get_int_or(sub, "x", 0);
    out->y = json_get_int_or(sub, "y", 0);
}

static void json_get_size(JsonObject *parent, const gchar *key, Size *out) {
    JsonObject *sub;

    out->width  = 0;
    out->height = 0;

    if (!json_object_has_member(parent, key))
        return;
    sub = json_object_get_object_member(parent, key);
    if (!sub)
        return;

    out->width  = json_get_int_or(sub, "width", 0);
    out->height = json_get_int_or(sub, "height", 0);
}

static void parse_marquee(JsonObject *root, MarqueeConfig *out) {
    JsonObject *m;

    out->content    = NULL;
    out->location.x = 0;
    out->location.y = 0;
    out->size.width  = 0;
    out->size.height = DEFAULT_MARQUEE_HEIGHT;
    out->font_size  = DEFAULT_MARQUEE_FONT_SIZE;
    out->font_style = g_strdup(DEFAULT_FONT_STYLE);
    out->color      = g_strdup("#FFFFFF");

    if (!json_object_has_member(root, "marquee"))
        return;
    m = json_object_get_object_member(root, "marquee");
    if (!m)
        return;

    out->content = json_dup_string_member(m, "content");
    json_get_point(m, "location", &out->location);
    json_get_size(m, "size", &out->size);
    out->font_size = json_get_int_or(m, "font_size", out->font_size);
    g_free(out->font_style);
    out->font_style = json_dup_string_or_default(m, "font_style", DEFAULT_FONT_STYLE);
    g_free(out->color);
    out->color = json_dup_string_or_default(m, "color", "#FFFFFF");
}

static void parse_clock(JsonObject *root, const gchar *key, ClockConfig *out,
                        const gchar *default_format) {
    JsonObject *c;

    out->format      = g_strdup(default_format);
    out->location.x  = 0;
    out->location.y  = 0;
    out->size.width  = 0;
    out->size.height = 0;
    out->font_size   = DEFAULT_CLOCK_FONT_SIZE;
    out->font_style  = g_strdup(DEFAULT_FONT_STYLE);
    out->color       = g_strdup(DEFAULT_COLOR);
    out->visibility  = TRUE;

    if (!json_object_has_member(root, key))
        return;
    c = json_object_get_object_member(root, key);
    if (!c)
        return;

    g_free(out->format);
    out->format = json_dup_string_or_default(c, "format", default_format);
    json_get_point(c, "location", &out->location);
    json_get_size(c, "size", &out->size);
    out->font_size = json_get_int_or(c, "font_size", out->font_size);
    g_free(out->font_style);
    out->font_style = json_dup_string_or_default(c, "font_style", DEFAULT_FONT_STYLE);
    g_free(out->color);
    out->color = json_dup_string_or_default(c, "color", DEFAULT_COLOR);
    out->visibility = json_get_bool_or(c, "visibility", TRUE);
}

static void parse_network(JsonObject *root, NetworkConfig *out) {
    JsonObject *n;

    out->ip_address = NULL;
    out->ws_port    = DEFAULT_WS_PORT;

    if (!json_object_has_member(root, "network"))
        return;
    n = json_object_get_object_member(root, "network");
    if (!n)
        return;

    out->ip_address = json_dup_string_member(n, "ip_address");
    out->ws_port    = json_get_int_or(n, "ws_port", DEFAULT_WS_PORT);
}

static void build_point(JsonBuilder *b, const gchar *key, const Point *p) {
    json_builder_set_member_name(b, key);
    json_builder_begin_object(b);
    json_builder_set_member_name(b, "x");
    json_builder_add_int_value(b, p->x);
    json_builder_set_member_name(b, "y");
    json_builder_add_int_value(b, p->y);
    json_builder_end_object(b);
}

static void build_size(JsonBuilder *b, const gchar *key, const Size *s) {
    json_builder_set_member_name(b, key);
    json_builder_begin_object(b);
    json_builder_set_member_name(b, "width");
    json_builder_add_int_value(b, s->width);
    json_builder_set_member_name(b, "height");
    json_builder_add_int_value(b, s->height);
    json_builder_end_object(b);
}

static void build_marquee(JsonBuilder *b, const MarqueeConfig *m) {
    json_builder_set_member_name(b, "marquee");
    json_builder_begin_object(b);

    json_builder_set_member_name(b, "content");
    json_builder_add_string_value(b, m->content ? m->content : "");
    build_point(b, "location", &m->location);
    build_size(b, "size", &m->size);
    json_builder_set_member_name(b, "font_size");
    json_builder_add_int_value(b, m->font_size);
    json_builder_set_member_name(b, "font_style");
    json_builder_add_string_value(b, m->font_style ? m->font_style : DEFAULT_FONT_STYLE);
    json_builder_set_member_name(b, "color");
    json_builder_add_string_value(b, m->color ? m->color : "#FFFFFF");

    json_builder_end_object(b);
}

static void build_clock(JsonBuilder *b, const gchar *key, const ClockConfig *c) {
    json_builder_set_member_name(b, key);
    json_builder_begin_object(b);

    json_builder_set_member_name(b, "format");
    json_builder_add_string_value(b, c->format ? c->format : "");
    build_point(b, "location", &c->location);
    build_size(b, "size", &c->size);
    json_builder_set_member_name(b, "font_size");
    json_builder_add_int_value(b, c->font_size);
    json_builder_set_member_name(b, "font_style");
    json_builder_add_string_value(b, c->font_style ? c->font_style : DEFAULT_FONT_STYLE);
    json_builder_set_member_name(b, "color");
    json_builder_add_string_value(b, c->color ? c->color : DEFAULT_COLOR);
    json_builder_set_member_name(b, "visibility");
    json_builder_add_boolean_value(b, c->visibility);

    json_builder_end_object(b);
}

static void build_network(JsonBuilder *b, const NetworkConfig *n) {
    json_builder_set_member_name(b, "network");
    json_builder_begin_object(b);

    json_builder_set_member_name(b, "ip_address");
    json_builder_add_string_value(b, n->ip_address ? n->ip_address : "");
    json_builder_set_member_name(b, "ws_port");
    json_builder_add_int_value(b, n->ws_port);

    json_builder_end_object(b);
}

gboolean config_loader_load_app_config(const char *file_path, AppConfig *app_config) {
    JsonParser *parser = NULL;
    JsonNode *root = NULL;
    JsonObject *obj = NULL;
    GError *error = NULL;

    if (!file_path || !app_config)
        return FALSE;

    /* zero scalars and pointers, then fill via parsers (which set their own defaults) */
    app_config->window_title = NULL;
    app_config->background = NULL;
    app_config->layout_file_path = NULL;
    app_config->css_file_path = NULL;
    app_config->editor_password = NULL;

    parser = json_parser_new();

    if (!json_parser_load_from_file(parser, file_path, &error)) {
        g_warning(
            "Cannot load config '%s': %s",
            file_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);
        g_object_unref(parser);

        return FALSE;
    }

    root = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        g_warning("Config root is not an object");
        g_object_unref(parser);

        return FALSE;
    }

    obj = json_node_get_object(root);

    app_config->window_title = json_dup_string_member(obj, "window_title");
    app_config->background = json_dup_string_member(obj, "background");
    app_config->layout_file_path = json_dup_string_member(obj, "layout_file_path");
    app_config->css_file_path = json_dup_string_member(obj, "css_file_path");
    app_config->editor_password = json_dup_string_member(obj, "editor_password");

    parse_marquee(obj, &app_config->marquee);
    parse_clock(obj, "time_zone", &app_config->time_zone, "hh:mm:ss");
    parse_clock(obj, "date_zone", &app_config->date_zone, "dd/mm/yy");
    parse_network(obj, &app_config->network);

    g_object_unref(parser);

    if (!app_config->background || !app_config->layout_file_path || !app_config->css_file_path) {
        g_warning("Missing required config fields");
        config_loader_free_app_config(app_config);

        return FALSE;
    }

    return TRUE;
}

gboolean config_loader_save_app_config(const char *file_path, const AppConfig *app_config) {
    JsonBuilder *builder = NULL;
    JsonGenerator *generator = NULL;
    JsonNode *root = NULL;
    GError *error = NULL;
    gboolean ok = FALSE;

    if (!file_path || !app_config)
        return FALSE;

    builder = json_builder_new();

    json_builder_begin_object(builder);
    if (app_config->window_title) {
        json_builder_set_member_name(builder, "window_title");
        json_builder_add_string_value(builder, app_config->window_title);
    }
    if (app_config->background) {
        json_builder_set_member_name(builder, "background");
        json_builder_add_string_value(builder, app_config->background);
    }
    if (app_config->layout_file_path) {
        json_builder_set_member_name(builder, "layout_file_path");
        json_builder_add_string_value(builder, app_config->layout_file_path);
    }
    if (app_config->css_file_path) {
        json_builder_set_member_name(builder, "css_file_path");
        json_builder_add_string_value(builder, app_config->css_file_path);
    }
    if (app_config->editor_password) {
        json_builder_set_member_name(builder, "editor_password");
        json_builder_add_string_value(builder, app_config->editor_password);
    }

    build_marquee(builder, &app_config->marquee);
    build_clock(builder, "time_zone", &app_config->time_zone);
    build_clock(builder, "date_zone", &app_config->date_zone);
    build_network(builder, &app_config->network);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);

    generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);

    if (!json_generator_to_file(generator, file_path, &error)) {
        g_warning(
            "Cannot save config '%s': %s",
            file_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);
        ok = FALSE;
    } else {
        ok = TRUE;
    }
    json_node_free(root);
    g_object_unref(generator);
    g_object_unref(builder);

    return ok;
}

void config_loader_free_app_config(AppConfig *config) {
    if (!config)
        return;

    g_clear_pointer(&config->window_title, g_free);
    g_clear_pointer(&config->background, g_free);
    g_clear_pointer(&config->layout_file_path, g_free);
    g_clear_pointer(&config->css_file_path, g_free);
    g_clear_pointer(&config->editor_password, g_free);

    g_clear_pointer(&config->marquee.content, g_free);
    g_clear_pointer(&config->marquee.font_style, g_free);
    g_clear_pointer(&config->marquee.color, g_free);

    g_clear_pointer(&config->time_zone.format, g_free);
    g_clear_pointer(&config->time_zone.font_style, g_free);
    g_clear_pointer(&config->time_zone.color, g_free);

    g_clear_pointer(&config->date_zone.format, g_free);
    g_clear_pointer(&config->date_zone.font_style, g_free);
    g_clear_pointer(&config->date_zone.color, g_free);

    g_clear_pointer(&config->network.ip_address, g_free);
}
