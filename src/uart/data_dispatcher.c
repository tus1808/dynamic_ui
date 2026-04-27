#include "uart/data_dispatcher.h"

#include <glib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

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

/* 0x01-0x09 → "01"-"09"  |  0x10-0x18 → "01."-"09."  |  else → "" */
static void decode_data_byte(guint8 code, gchar *text, gsize text_len) {
    if (code >= 0x01 && code <= 0x09) {
        g_snprintf(text, text_len, "%02d", (int)code);
    } else if (code >= 0x10 && code <= 0x18) {
        g_snprintf(text, text_len, "%02d.", (int)(code - 0x0F));
    } else {
        text[0] = '\0';
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
        gchar text[8];

        item_widget = g_ptr_array_index(state->value_items, i);
        if (!item_widget)
            continue;

        decode_data_byte(frame[UART_HEADER_SIZE + i], text, sizeof(text));
        ui_value_item_set_value(item_widget, text);
    }
}

/* Body layout: [0]=year since 2000  [1]=month(1-12)  [2]=day  [3]=hour  [4]=min  [5]=sec */
static void data_dispatcher_apply_time(const guint8 *frame) {
    const guint8 *body = frame + UART_HEADER_SIZE;
    struct tm t = {0};
    struct timeval tv;
    time_t epoch;

    t.tm_year  = (int)body[0] + 100; /* years since 1900 */
    t.tm_mon   = (int)body[1] - 1;   /* 0-indexed */
    t.tm_mday  = (int)body[2];
    t.tm_hour  = (int)body[3];
    t.tm_min   = (int)body[4];
    t.tm_sec   = (int)body[5];
    t.tm_isdst = -1;

    epoch = mktime(&t);
    if (epoch == (time_t)-1) {
        g_warning("[UART] Time frame: invalid time values");
        return;
    }

    tv.tv_sec  = epoch;
    tv.tv_usec = 0;

    if (settimeofday(&tv, NULL) != 0) {
        g_warning("[UART] Failed to set system time (requires root)");
    } else {
        g_print(
            "[UART] System time updated: %04d-%02d-%02d %02d:%02d:%02d\n",
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec
        );
    }
}

static gboolean data_dispatcher_apply_on_main_thread(gpointer user_data) {
    DispatchJob *job = user_data;
    guint8 frame_type;

    if (!job || !job->dispatcher) {
        g_free(job);
        return G_SOURCE_REMOVE;
    }

    data_dispatcher_print_frame_to_terminal(job->frame);

    frame_type = job->frame[1];

    switch (frame_type) {
    case UART_FRAME_TYPE_DATA:
        data_dispatcher_update_ui(job->dispatcher, job->frame);
        break;
    case UART_FRAME_TYPE_TIME:
        data_dispatcher_apply_time(job->frame);
        break;
    default:
        g_print("[UART] Unknown frame type: 0x%02X\n", frame_type);
        break;
    }

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