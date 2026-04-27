#pragma once

#define UART_DEFAULT_DEVICE "/dev/serial0"
#define UART_FRAME_SIZE 136
#define UART_HEADER_SIZE 8
#define UART_BODY_SIZE 128

#define UART_FRAME_TYPE_DATA 0x01
#define UART_FRAME_TYPE_TIME 0x02
