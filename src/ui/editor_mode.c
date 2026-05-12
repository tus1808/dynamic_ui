#include "ui/editor_mode.h"

#include <glib.h>

#include "app/controller.h"
#include "app/state.h"
#include "config/loader.h"
#include "ui/canvas.h"
#include "ui/overlay.h"
#include "ui/value_item.h"

static gint next_index_for_state(AppState *state) {
    gint max_idx = -1;

    if (!state || !state->layout_items)
        return 0;

    for (guint i = 0; i < state->layout_items->len; i++) {
        LayoutItem *li = g_ptr_array_index(state->layout_items, i);
        if (li && li->index > max_idx)
            max_idx = li->index;
    }
    return max_idx + 1;
}

static LayoutItem *editor_mode_create_default_item(guint ordinal, gint index) {
    LayoutItem *item = g_new0(LayoutItem, 1);

    item->_id        = g_strdup_printf("item %u", ordinal);
    item->index      = index;
    item->location.x = 100 + ((ordinal - 1) % 5) * 160;
    item->location.y = 100 + ((ordinal - 1) / 5) * 70;
    item->width      = 140;
    item->height     = 40;
    item->value      = g_strdup("--");

    item->font_size              = 0;
    item->font_style             = g_strdup("bold");
    item->font_color             = g_strdup("#000000");
    item->background_color       = g_strdup("#FFFFFF");
    item->background_transparent = TRUE;

    return item;
}

EditorMode *editor_mode_new(AppController *controller) {
    EditorMode *mode = g_new0(EditorMode, 1);
    mode->controller = controller;

    return mode;
}

void editor_mode_free(EditorMode *mode) {
    if (!mode)
        return;

    g_free(mode);
}

void editor_mode_enter(EditorMode *mode) {
    AppState *state;

    if (!mode || !mode->controller || !mode->controller->state)
        return;

    state = mode->controller->state;
    state->current_mode = APP_MODE_EDITOR;

    if (state->canvas)
        ui_canvas_set_interactive(state->canvas, TRUE);

    if (state->value_items) {
        for (guint i = 0; i < state->value_items->len; i++)
            ui_value_item_set_read_mode(g_ptr_array_index(state->value_items, i), FALSE);
    }

    g_print("[MODE] Enter EDITOR mode\n");
}

void editor_mode_exit(EditorMode *mode) {
    if (!mode || !mode->controller || !mode->controller->state)
        return;

    if (mode->controller->state->canvas) {
        ui_canvas_clear_selection(mode->controller->state->canvas);
        ui_canvas_set_interactive(mode->controller->state->canvas, FALSE);
    }
}

void editor_mode_add_one_item(EditorMode *mode) {
    AppState *state;
    LayoutItem *item;
    GtkWidget *widget;
    gint next_index;

    if (!mode || !mode->controller || !mode->controller->state)
        return;

    state = mode->controller->state;

    if (!state->layout_items)
        state->layout_items = g_ptr_array_new();

    if (!state->value_items)
        state->value_items = g_ptr_array_new();

    next_index = next_index_for_state(state);
    item = editor_mode_create_default_item(state->layout_items->len + 1, next_index);
    g_ptr_array_add(state->layout_items, item);

    widget = ui_canvas_add_item(state->canvas, item);
    if (widget) {
        g_ptr_array_add(state->value_items, widget);
        ui_value_item_apply_font_size(widget, item->height);
        ui_value_item_set_read_mode(widget, FALSE);
        ui_canvas_select_only(state->canvas, widget);
    }
}

void editor_mode_add_many_items(EditorMode *mode) {
    for (int i = 0; i < 5; i++)
        editor_mode_add_one_item(mode);
}

void editor_mode_delete_selected_item(EditorMode *mode) {
    AppState *state;
    GPtrArray *selected_items;

    if (!mode || !mode->controller || !mode->controller->state)
        return;

    state = mode->controller->state;
    selected_items = ui_canvas_get_selected_items(state->canvas);

    if (!selected_items || selected_items->len == 0)
        return;

    while (selected_items->len > 0) {
        GtkWidget *widget = g_ptr_array_index(selected_items, 0);
        LayoutItem *item = ui_value_item_get_layout_item(widget);

        if (state->value_items) {
            for (guint i = 0; i < state->value_items->len; i++) {
                if (g_ptr_array_index(state->value_items, i) == widget) {
                    g_ptr_array_remove_index(state->value_items, i);
                    break;
                }
            }
        }

        if (state->layout_items && item) {
            for (guint i = 0; i < state->layout_items->len; i++) {
                if (g_ptr_array_index(state->layout_items, i) == item) {
                    g_ptr_array_remove_index(state->layout_items, i);
                    break;
                }
            }
        }

        ui_canvas_remove_item(state->canvas, widget);
    }
}

static gint style_index_for_name(const gchar *name) {
    if (!name) return 0;
    if (g_ascii_strcasecmp(name, "normal") == 0) return 0;
    if (g_ascii_strcasecmp(name, "bold") == 0) return 1;
    if (g_ascii_strcasecmp(name, "italic") == 0) return 2;
    if (g_ascii_strcasecmp(name, "bold_italic") == 0) return 3;
    return 0;
}

static const gchar *style_name_for_index(gint idx) {
    switch (idx) {
        case 1: return "bold";
        case 2: return "italic";
        case 3: return "bold_italic";
        default: return "normal";
    }
}

static gchar *gdk_rgba_to_hex(const GdkRGBA *rgba) {
    return g_strdup_printf("#%02X%02X%02X",
                           (int)(rgba->red   * 255.0),
                           (int)(rgba->green * 255.0),
                           (int)(rgba->blue  * 255.0));
}

void editor_mode_show_info_box(EditorMode *mode) {
    AppState *state;
    GPtrArray *selected_items;
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *id_entry;
    GtkWidget *x_spin, *y_spin, *w_spin, *h_spin;
    GtkWidget *index_spin, *fs_spin;
    GtkWidget *style_combo;
    GtkWidget *font_color_btn, *bg_color_btn;
    GtkWidget *transparent_check;
    LayoutItem *item;
    GtkWidget *widget;
    GdkRGBA rgba_font, rgba_bg;
    int row = 0;

    if (!mode || !mode->controller || !mode->controller->state)
        return;

    state = mode->controller->state;
    selected_items = ui_canvas_get_selected_items(state->canvas);

    if (!selected_items || selected_items->len != 1)
        return;

    widget = g_ptr_array_index(selected_items, 0);
    item = ui_value_item_get_layout_item(widget);
    if (!item)
        return;

    dialog = gtk_dialog_new_with_buttons(
        "Item Info",
        GTK_WINDOW(state->window),
        GTK_DIALOG_MODAL,
        "_Cancel",
        GTK_RESPONSE_CANCEL,
        "_Apply",
        GTK_RESPONSE_OK,
        NULL
    );

    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_container_add(GTK_CONTAINER(content), grid);

    id_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(id_entry), item->_id ? item->_id : "");

    index_spin = gtk_spin_button_new_with_range(0, 999, 1);
    x_spin = gtk_spin_button_new_with_range(0, 1920, 1);
    y_spin = gtk_spin_button_new_with_range(0, 1920, 1);
    w_spin = gtk_spin_button_new_with_range(60, 1920, 10);
    h_spin = gtk_spin_button_new_with_range(30, 1920, 10);
    fs_spin = gtk_spin_button_new_with_range(0, 200, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(index_spin), item->index);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_spin), item->location.x);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_spin), item->location.y);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w_spin), item->width);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(h_spin), item->height);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(fs_spin), item->font_size);

    style_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(style_combo), "normal");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(style_combo), "bold");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(style_combo), "italic");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(style_combo), "bold_italic");
    gtk_combo_box_set_active(GTK_COMBO_BOX(style_combo), style_index_for_name(item->font_style));

    gdk_rgba_parse(&rgba_font, item->font_color ? item->font_color : "#000000");
    gdk_rgba_parse(&rgba_bg,   item->background_color ? item->background_color : "#FFFFFF");
    font_color_btn = gtk_color_button_new_with_rgba(&rgba_font);
    bg_color_btn   = gtk_color_button_new_with_rgba(&rgba_bg);

    transparent_check = gtk_check_button_new_with_label("Transparent background");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(transparent_check),
                                  item->background_transparent);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID"),       0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_entry,                  1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Index"),    0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), index_spin,                1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("X"),        0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), x_spin,                    1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Y"),        0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), y_spin,                    1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Width"),    0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w_spin,                    1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Height"),   0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), h_spin,                    1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Font size (0 = auto)"), 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), fs_spin,                   1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Font style"), 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), style_combo,               1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Font color"), 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), font_color_btn,            1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Background"), 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bg_color_btn,              1, row, 1, 1); row++;
    gtk_grid_attach(GTK_GRID(grid), transparent_check,         0, row, 2, 1); row++;

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        GdkRGBA picked_font, picked_bg;
        gchar *style_text;

        g_free(item->_id);
        item->_id = g_strdup(gtk_entry_get_text(GTK_ENTRY(id_entry)));

        item->index = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(index_spin));
        item->location.x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(x_spin));
        item->location.y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(y_spin));
        item->width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w_spin));
        item->height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(h_spin));
        item->font_size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(fs_spin));

        g_free(item->font_style);
        style_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(style_combo));
        item->font_style = g_strdup(
            style_text ? style_text : style_name_for_index(
                gtk_combo_box_get_active(GTK_COMBO_BOX(style_combo))
            )
        );
        g_free(style_text);

        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(font_color_btn), &picked_font);
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(bg_color_btn),   &picked_bg);
        g_free(item->font_color);
        item->font_color = gdk_rgba_to_hex(&picked_font);
        g_free(item->background_color);
        item->background_color = gdk_rgba_to_hex(&picked_bg);

        item->background_transparent =
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(transparent_check));

        gtk_fixed_move(GTK_FIXED(state->canvas), widget, item->location.x, item->location.y);
        gtk_widget_set_size_request(widget, item->width, item->height);
        ui_value_item_refresh_style(widget);
        ui_value_item_apply_font_size(widget, item->height);
        ui_value_item_set_read_mode(widget, FALSE);
        ui_value_item_set_value(widget, item->value);
    }

    gtk_widget_destroy(dialog);
}

void editor_mode_change_background(EditorMode *mode) {
    AppState *state;
    GtkWidget *dialog;
    gint response;
    char *file_path = NULL;

    if (!mode || !mode->controller || !mode->controller->state)
        return;

    state = mode->controller->state;

    dialog = gtk_file_chooser_dialog_new(
        "Choose Background",
        GTK_WINDOW(state->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel",
        GTK_RESPONSE_CANCEL,
        "_Open",
        GTK_RESPONSE_ACCEPT,
        NULL
    );

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images & Videos");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.gif");
    gtk_file_filter_add_pattern(filter, "*.mp4");
    gtk_file_filter_add_pattern(filter, "*.mkv");
    gtk_file_filter_add_pattern(filter, "*.avi");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT)
        file_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    gtk_widget_destroy(dialog);

    if (!file_path)
        return;

    if (ui_overlay_set_background(state->overlay, file_path)) {
        app_state_set_background_path(state, file_path);
        config_loader_save_app_config(CONFIG_FILE_PATH, &state->config);
        g_print("[EDITOR] Background changed to: %s\n", file_path);
    }

    g_free(file_path);
}
