#include "ui/editor_mode.h"

#include <glib.h>

#include "app/controller.h"
#include "app/state.h"
#include "config/loader.h"
#include "ui/canvas.h"
#include "ui/overlay.h"
#include "ui/value_item.h"

static LayoutItem *editor_mode_create_default_item(guint index) {
    LayoutItem *item = g_new0(LayoutItem, 1);

    item->_id = g_strdup_printf("item %u", index);
    item->x = 100 + ((index - 1) % 5) * 160;
    item->y = 100 + ((index - 1) / 5) * 70;
    item->width = 140;
    item->height = 40;
    item->value = g_strdup("--");

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

    if (!mode || !mode->controller || !mode->controller->state)
        return;

    state = mode->controller->state;

    if (!state->layout_items)
        state->layout_items = g_ptr_array_new();

    if (!state->value_items)
        state->value_items = g_ptr_array_new();

    item = editor_mode_create_default_item(state->layout_items->len + 1);
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

void editor_mode_show_info_box(EditorMode *mode) {
    AppState *state;
    GPtrArray *selected_items;
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *id_entry;
    GtkWidget *x_spin, *y_spin, *w_spin, *h_spin;
    LayoutItem *item;
    GtkWidget *widget;

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

    x_spin = gtk_spin_button_new_with_range(0, 1920, 1);
    y_spin = gtk_spin_button_new_with_range(0, 1920, 1);
    w_spin = gtk_spin_button_new_with_range(60, 1920, 10);
    h_spin = gtk_spin_button_new_with_range(30, 1920, 10);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_spin), item->x);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_spin), item->y);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w_spin), item->width);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(h_spin), item->height);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), id_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("X"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), x_spin, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Y"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), y_spin, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Width"), 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w_spin, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Height"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), h_spin, 1, 4, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        g_free(item->_id);

        item->_id = g_strdup(gtk_entry_get_text(GTK_ENTRY(id_entry)));
        item->x = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(x_spin));
        item->y = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(y_spin));
        item->width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w_spin));
        item->height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(h_spin));

        gtk_fixed_move(GTK_FIXED(state->canvas), widget, item->x, item->y);
        gtk_widget_set_size_request(widget, item->width, item->height);
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