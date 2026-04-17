#include "uart/port.h"

#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct _UartPort {
    gchar *device_path;
    int baudrate;
    int fd;
    GThread *thread;
    gboolean running;

    UartFrameCallback callback;
    gpointer user_data;
};

static speed_t uart_port_map_baudrate(int baudrate) {
    switch (baudrate) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
#ifdef B230400
    case 230400:
        return B230400;
#endif
    default:
        return B9600;
    }
}

static gboolean uart_port_configure(int fd, int baudrate) {
    struct termios tty;
    speed_t speed;

    if (tcgetattr(fd, &tty) != 0)
        return FALSE;

    speed = uart_port_map_baudrate(baudrate);

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
#endif

    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcflush(fd, TCIFLUSH) != 0)
        return FALSE;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        return FALSE;

    return TRUE;
}

static int uart_port_open_fd(const gchar *device_path, int baudrate) {
    int fd;

    if (!device_path)
        return -1;

    fd = open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
        return -1;

    if (!uart_port_configure(fd, baudrate)) {
        close(fd);
        return -1;
    }

    return fd;
}

static gpointer uart_port_read_thread(gpointer data) {
    UartPort *port = data;
    guint8 frame[UART_FRAME_SIZE];
    gsize filled = 0;

    while (port && port->running) {
        ssize_t n;

        if (port->fd < 0)
            break;

        n = read(port->fd, frame + filled, UART_FRAME_SIZE - filled);

        if (n > 0) {
            filled += (gsize)n;

            if (filled == UART_FRAME_SIZE) {
                g_print("[UART RX] complete frame: %d bytes\n", UART_FRAME_SIZE);
                if (port->callback)

                    port->callback(frame, UART_FRAME_SIZE, port->user_data);

                filled = 0;
            }

            continue;
        }

        if (n == 0 || errno == EAGAIN || errno == EWOULDBLOCK) {
            g_usleep(2000);
            continue;
        }

        g_print("[UART RX] read error: %s\n", g_strerror(errno));
        break;
    }

    port->running = FALSE;
    return NULL;
}

UartPort *uart_port_new(int baudrate, UartFrameCallback callback, gpointer user_data) {
    UartPort *port = g_new0(UartPort, 1);

    port->baudrate = baudrate;
    port->fd = -1;
    port->callback = callback;
    port->user_data = user_data;

    return port;
}

void uart_port_free(UartPort *port) {
    if (!port)
        return;

    uart_port_stop(port);
    uart_port_close(port);

    g_free(port->device_path);
}

gboolean uart_port_open(UartPort *port, const gchar *device_path) {
    int fd;
    const gchar *path;

    if (!port)
        return FALSE;

    path = (device_path && *device_path) ? device_path : UART_DEFAULT_DEVICE;

    uart_port_close(port);

    fd = uart_port_open_fd(device_path, port->baudrate);
    if (fd < 0)
        return FALSE;

    port->fd = fd;
    port->device_path = g_strdup(device_path);

    g_print("[UART] connected to %s\n", port->device_path);
    return TRUE;
}

void uart_port_close(UartPort *port) {
    if (!port)
        return;

    if (port->fd >= 0) {
        close(port->fd);
        port->fd = -1;
    }

    g_clear_pointer(&port->device_path, g_free);
}

gboolean uart_port_start(UartPort *port) {
    if (!port || port->fd < 0)
        return FALSE;

    if (port->running)
        return TRUE;

    port->running = TRUE;
    port->thread = g_thread_new("uart-read-thread", uart_port_read_thread, port);

    if (!port->thread) {
        port->running = FALSE;
        return FALSE;
    }

    return TRUE;
}

void uart_port_stop(UartPort *port) {
    if (!port)
        return;

    if (!port->running && !port->thread)
        return;

    port->running = FALSE;

    if (port->thread) {
        g_thread_join(port->thread);
        port->thread = NULL;
    }
}