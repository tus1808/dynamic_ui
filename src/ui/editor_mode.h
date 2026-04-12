#ifndef EDITOR_MODE_H
#define EDITOR_MODE_H

typedef struct _AppController AppController;

typedef struct {
    AppController *controller;
} EditorMode;

EditorMode *editor_mode_new(AppController *controller);
void editor_mode_free(EditorMode *mode);

void editor_mode_enter(EditorMode *mode);
void editor_mode_exit(EditorMode *mode);

void editor_mode_add_one_item(EditorMode *mode);
void editor_mode_add_many_items(EditorMode *mode);
void editor_mode_delete_selected_item(EditorMode *mode);
void editor_mode_show_info_box(EditorMode *mode);

void editor_mode_change_background(EditorMode *mode);

#endif