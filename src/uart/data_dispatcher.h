#pragma once

#include <glib.h>

#include "app/state.h"

G_BEGIN_DECLS

typedef struct _DataDispatcher DataDispatcher;

DataDispatcher *data_dispatcher_new(AppState *state);
void data_dispatcher_free(DataDispatcher *dispatcher);

void data_dispatcher_submit_frame(
    DataDispatcher *dispatcher, const guint8 *frame, gsize frame_size
);

G_END_DECLS