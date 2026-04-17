#pragma once

#include <glib.h>
#include <stddef.h>
#include <stdint.h>

#include "common/contants.h"

G_BEGIN_DECLS

typedef void (*UartFrameCallback)(const guint8 *frame, gsize frame_size, gpointer user_data);

typedef struct _UartPort UartPort;

UartPort *uart_port_new(int baudrate, UartFrameCallback callback, gpointer user_date);
void uart_port_free(UartPort *port);

gboolean uart_port_find_working_device(UartPort *port);

gboolean uart_port_open(UartPort *port, const gchar *device_path);
void uart_port_close(UartPort *port);

gboolean uart_port_start(UartPort *port);
void uart_port_stop(UartPort *port);

const gchar *uart_port_get_device_path(const UartPort *port);

G_END_DECLS