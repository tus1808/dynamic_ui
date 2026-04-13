#include "uart/parser.h"

#include "string.h"

gboolean uart_parser_parse_frame(const uint8_t *raw, gsize raw_len, UartFrame *out_frame) {
    if (!raw || !out_frame)
        return FALSE;

    if (raw_len != UART_FRAME_SIZE)
        return FALSE;

    memcpy(out_frame->header, raw, UART_HEADER_SIZE);
    memcpy(out_frame->body, raw + UART_HEADER_SIZE, UART_BODY_SIZE);

    return TRUE;
}