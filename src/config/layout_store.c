#include "config/layout_store.h"

#include <json-glib/json-glib.h>
#include <glib.h>

#define DEFAULT_FONT_STYLE       "bold"
#define DEFAULT_FONT_COLOR       "#000000"
#define DEFAULT_BACKGROUND_COLOR "#FFFFFF"

static LayoutItem *layout_item_new(void) { return g_new0(LayoutItem, 1); }

static gchar *dup_string(JsonObject *obj, const gchar *key) {
    if (!json_object_has_member(obj, key))
        return NULL;

    return g_strdup(json_object_get_string_member(obj, key));
}

static gchar *dup_string_or_default(JsonObject *obj, const gchar *key, const gchar *fallback) {
    gchar *out = dup_string(obj, key);
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

static void read_location(JsonObject *obj, Point *out, gint legacy_x, gint legacy_y) {
    out->x = legacy_x;
    out->y = legacy_y;

    if (!json_object_has_member(obj, "location"))
        return;

    JsonObject *loc = json_object_get_object_member(obj, "location");
    if (!loc)
        return;

    out->x = json_get_int_or(loc, "x", legacy_x);
    out->y = json_get_int_or(loc, "y", legacy_y);
}

static void add_item_to_builder(JsonBuilder *builder, const LayoutItem *item) {
    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "_id");
    json_builder_add_string_value(builder, item->_id ? item->_id : "");

    json_builder_set_member_name(builder, "index");
    json_builder_add_int_value(builder, item->index);

    json_builder_set_member_name(builder, "location");
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "x");
    json_builder_add_int_value(builder, item->location.x);
    json_builder_set_member_name(builder, "y");
    json_builder_add_int_value(builder, item->location.y);
    json_builder_end_object(builder);

    json_builder_set_member_name(builder, "width");
    json_builder_add_int_value(builder, item->width);

    json_builder_set_member_name(builder, "height");
    json_builder_add_int_value(builder, item->height);

    json_builder_set_member_name(builder, "value");
    json_builder_add_string_value(builder, item->value ? item->value : "");

    json_builder_set_member_name(builder, "font_size");
    json_builder_add_int_value(builder, item->font_size);

    json_builder_set_member_name(builder, "font_style");
    json_builder_add_string_value(builder, item->font_style ? item->font_style : DEFAULT_FONT_STYLE);

    json_builder_set_member_name(builder, "font_color");
    json_builder_add_string_value(builder, item->font_color ? item->font_color : DEFAULT_FONT_COLOR);

    json_builder_set_member_name(builder, "background_color");
    json_builder_add_string_value(
        builder, item->background_color ? item->background_color : DEFAULT_BACKGROUND_COLOR
    );

    json_builder_set_member_name(builder, "background_transparent");
    json_builder_add_boolean_value(builder, item->background_transparent);

    json_builder_end_object(builder);
}

gboolean layout_store_save(const char *file_path, GPtrArray *items) {
    JsonBuilder *builder = NULL;
    JsonGenerator *generator = NULL;
    JsonNode *root = NULL;
    GError *error = NULL;
    gboolean ok = FALSE;

    if (!file_path || !items)
        return FALSE;

    builder = json_builder_new();
    json_builder_begin_array(builder);
    for (guint i = 0; i < items->len; i++) {
        LayoutItem *item = g_ptr_array_index(items, i);
        if (!item)
            continue;

        add_item_to_builder(builder, item);
    }
    json_builder_end_array(builder);

    root = json_builder_get_root(builder);

    generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_set_pretty(generator, TRUE);

    ok = json_generator_to_file(generator, file_path, &error);
    if (!ok) {
        g_warning(
            "Failed to save layout to file '%s': %s",
            file_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);
    }

    if (root)
        json_node_free(root);

    if (generator)
        g_object_unref(generator);

    if (builder)
        g_object_unref(builder);

    return ok;
}

void layout_store_free_item(LayoutItem *item) {
    if (!item)
        return;

    g_clear_pointer(&item->_id, g_free);
    g_clear_pointer(&item->value, g_free);
    g_clear_pointer(&item->font_style, g_free);
    g_clear_pointer(&item->font_color, g_free);
    g_clear_pointer(&item->background_color, g_free);

    g_free(item);
}

static gint compare_items_by_index(gconstpointer a, gconstpointer b) {
    const LayoutItem *ia = *(const LayoutItem * const *)a;
    const LayoutItem *ib = *(const LayoutItem * const *)b;
    if (ia->index < ib->index) return -1;
    if (ia->index > ib->index) return 1;
    return 0;
}

GPtrArray *layout_store_load(const char *file_path) {
    JsonParser *parser = NULL;
    JsonNode *root = NULL;
    JsonArray *array = NULL;
    GPtrArray *items = NULL;
    GError *error = NULL;
    guint i = 0;

    if (!file_path)
        return NULL;

    parser = json_parser_new();
    if (!json_parser_load_from_file(parser, file_path, &error)) {
        g_warning(
            "Cannot load layout '%s': %s",
            file_path,
            error ? error->message : "unknown error"
        );
        g_clear_error(&error);
        g_object_unref(parser);

        return NULL;
    }

    root = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_ARRAY(root)) {
        g_warning("Layout root is not an array");
        g_object_unref(parser);

        return NULL;
    }

    array = json_node_get_array(root);
    items = g_ptr_array_new_with_free_func((GDestroyNotify)layout_store_free_item);

    for (i = 0; i < json_array_get_length(array); i++) {
        JsonObject *obj = json_array_get_object_element(array, i);
        LayoutItem *item = layout_item_new();

        item->_id   = dup_string(obj, "_id");
        item->value = dup_string(obj, "value");
        item->index = json_get_int_or(obj, "index", (gint)i);

        gint legacy_x = json_get_int_or(obj, "x", 0);
        gint legacy_y = json_get_int_or(obj, "y", 0);
        read_location(obj, &item->location, legacy_x, legacy_y);

        item->width  = json_get_int_or(obj, "width", 140);
        item->height = json_get_int_or(obj, "height", 40);

        item->font_size  = json_get_int_or(obj, "font_size", 0);
        item->font_style = dup_string_or_default(obj, "font_style", DEFAULT_FONT_STYLE);
        item->font_color = dup_string_or_default(obj, "font_color", DEFAULT_FONT_COLOR);
        item->background_color =
            dup_string_or_default(obj, "background_color", DEFAULT_BACKGROUND_COLOR);
        item->background_transparent = json_get_bool_or(obj, "background_transparent", TRUE);

        g_ptr_array_add(items, item);
    }

    g_ptr_array_sort(items, compare_items_by_index);

    g_object_unref(parser);

    return items;
}
