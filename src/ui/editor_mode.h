#ifndef EDITOR_MODE_H
#define EDITOR_MODE_H

typedef struct _AppController AppController;

typedef struct
{
  AppController *controller;
} EditorMode;

EditorMode *editor_mode_new(AppController *controller);
void editor_mode_free(EditorMode *mode);

void editor_mode_enter(EditorMode *mode);
void editor_mode_exit(EditorMode *mode);

#endif