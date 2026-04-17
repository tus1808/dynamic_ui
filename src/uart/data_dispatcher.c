#include "uart/data_dispatcher.h"

#include <glib.h>
#include <string.h>

#include "common/contants.h"
#include "ui/value_item.h"

typedef struct {
    struct _DataDispatcher *dispatcher;
    guint8 frame[UART_FRAME_SIZE];
    gsize frame_size;
} DispatchJob;

struct _DataDispatcher {
    AppState *state;
};

static void data_dispatcher_print_frame_to_terminal(const guint8 *frame) {
    guint i;

    if (!frame)
        return;

    g_print("\n[UART] Frame received (%d bytes)\n", UART_FRAME_SIZE);

    g_print("[UART] Header: ");
    for (i = 0; i < UART_HEADER_SIZE; i++) {
        g_print("%02X ", frame[i]);
    }
    g_print("\n");

    g_print("[UART] Body:\n");
    for (i = 0; i < UART_BODY_SIZE; i++) {
        g_print("%02X ", frame[UART_HEADER_SIZE + i]);

        if (((i + 1) % 16) == 0)
            g_print("\n");
    }
}

static void data_dispatcher_update_ui(DataDispatcher *dispatcher, const guint8 *frame) {
    AppState *state;
    guint visible_count;
    guint i;

    if (!dispatcher || !dispatcher->state || !frame)
        return;

    state = dispatcher->state;

    if (!state->value_items)
        return;

    visible_count = MIN((guint)UART_BODY_SIZE, state->value_items->len);

    for (i = 0; i < visible_count; i++) {
        GtkWidget *item_widget;
        guint8 value;
        gchar text[8];

        item_widget = g_ptr_array_index(state->value_items, i);
        if (!item_widget)
            continue;

        value = frame[UART_HEADER_SIZE + i];
        g_snprintf(text, sizeof(text), "02%X", value);

        ui_value_item_set_value(item_widget, text);
    }
}

static gboolean data_dispatcher_apply_on_main_thread(gpointer user_data) {
    DispatchJob *job = user_data;

    if (!job || !job->dispatcher) {
        g_free(job);
        return G_SOURCE_REMOVE;
    }

    data_dispatcher_print_frame_to_terminal(job->frame);
    data_dispatcher_update_ui(job->dispatcher, job->frame);

    g_free(job);
    return G_SOURCE_REMOVE;
}

DataDispatcher *data_dispatcher_new(AppState *state) {
    DataDispatcher *dispatcher;

    if (!state)
        return NULL;

    dispatcher = g_new0(DataDispatcher, 1);
    dispatcher->state = state;

    return dispatcher;
}

void data_dispatcher_free(DataDispatcher *dispatcher) {
    if (!dispatcher)
        return;

    g_free(dispatcher);
}

void data_dispatcher_submit_frame(
    DataDispatcher *dispatcher, const guint8 *frame, gsize frame_size
) {
    DispatchJob *job;

    if (!dispatcher || !dispatcher->state || !frame || frame_size == 0)
        return;

    if (frame_size != UART_FRAME_SIZE) {
        g_print("[UART] ignored frame with invalid size: %zu\n", frame_size);

        return;
    }

    job = g_new0(DispatchJob, 1);
    job->dispatcher = dispatcher;
    job->frame_size = frame_size;

    memcpy(job->frame, frame, frame_size);

    g_idle_add(data_dispatcher_apply_on_main_thread, job);
}