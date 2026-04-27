#include "loader.h"
#include <json-glib/json-glib.h>

static gchar *json_dup_string_member(JsonObject *obj, const gchar *key) {
    if (!json_object_has_member(obj, key))
        return NULL;

    return g_strdup(json_object_get_string_member(obj, key));
}

gboolean config_loader_load_app_config(const char *file_path, AppConfig *app_config) {
    JsonParser *parser = NULL;
    JsonNode *root = NULL;
    JsonObject *obj = NULL;
    GError *error = NULL;

    if (!file_path || !app_config)
        return FALSE;

    app_config->window_title = NULL;
    app_config->background = NULL;
    app_config->layout_file_path = NULL;
    app_config->css_file_path = NULL;
    app_config->editor_password = NULL;
    app_config->marquee_content = NULL;

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
    app_config->marquee_content = json_dup_string_member(obj, "marquee_content");

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
    if (app_config->marquee_content) {
        json_builder_set_member_name(builder, "marquee_content");
        json_builder_add_string_value(builder, app_config->marquee_content);
    }
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
    g_clear_pointer(&config->marquee_content, g_free);
}