#ifndef UART_PARSER_H
#define UART_PARSER_H

#include <glib.h>
#include <stdint.h>

#include "common/contants.h"

typedef struct {
    uint8_t header[UART_HEADER_SIZE];
    uint8_t body[UART_BODY_SIZE];
} UartFrame;

gboolean uart_parser_parse_frame(const uint8_t *raw, gsize raw_len, UartFrame *outframe);

#endif