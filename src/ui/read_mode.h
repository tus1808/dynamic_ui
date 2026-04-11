#ifndef READ_MODE_H
#define READ_MODE_H

typedef struct _AppController AppController;

typedef struct {
    AppController *controller;
} ReadMode;

ReadMode *read_mode_new(AppController *controller);
void read_mode_free(ReadMode *mode);

void read_mode_enter(ReadMode *mode);
void read_mode_exit(ReadMode *mode);

#endif