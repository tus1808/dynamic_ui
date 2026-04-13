#include "uart/port.h"

#include <glib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <poll.h>

struct _UartPort {
    gchar *device_path;
    int baudrate;
    int fd;
    GThread *thread;
    gboolean running;

    UartFrameCallback callback;
    gpointer user_data;
};

static const gchar *UART_CANDIDATE_PORTS[] = {
    "/dev/ttyS0",
    "/dev/ttyS1",
    "/dev/ttyS2",
    "/dev/ttyS3",
    "/dev/ttyS4",
    "/dev/ttyS5",
    "/dev/ttyS6",
    "/dev/ttyS7",
    "/dev/ttyUSB0",
    "/dev/ttyUSB1",
    "/dev/ttyUSB2",
    "/dev/ttyUSB3",
    "/dev/ttyACM0",
    "/dev/ttyACM1",
    "/dev/ttyACM2",
    "/dev/ttyACM3",
    NULL
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
    // tty.c_cflag &= ~CRTSCTS;

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

static gboolean uart_port_wait_for_signal(int fd, int timeout_ms) {
    struct pollfd pfd;
    guint8 probe[8];
    int poll_result;
    ssize_t bytes_read;

    if (fd < 0)
        return FALSE;

    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    poll_result = poll(&pfd, 1, timeout_ms);
    if (poll_result <= 0)
        return FALSE;

    if ((pfd.revents & POLLIN) == 0)
        return FALSE;

    bytes_read = read(fd, probe, sizeof(probe));
    if (bytes_read <= 0)
        return FALSE;

    return TRUE;
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
                if (port->callback)
                    port->callback(frame, UART_FRAME_SIZE, port->user_data);

                filled = 0;
            }

            continue;
        }

        if (n == 0) {
            g_usleep(2000);
            continue;
        }

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

    if (!port || !device_path)
        return FALSE;

    uart_port_close(port);

    fd = uart_port_open_fd(device_path, port->baudrate);
    if (fd < 0)
        return FALSE;

    port->fd = fd;
    port->device_path = g_strdup(device_path);

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

gboolean uart_port_find_working_device(UartPort *port) {
    int i;

    if (!port)
        return FALSE;

    for (i = 0; UART_CANDIDATE_PORTS[i] != NULL; i++) {
        const gchar *candidate = UART_CANDIDATE_PORTS[i];
        int fd = uart_port_open_fd(candidate, port->baudrate);

        if (fd < 0)
            continue;

        if (uart_port_wait_for_signal(fd, UART_DETECT_TIMEOUT_MS)) {
            uart_port_close(port);
            port->fd = fd;
            port->device_path = g_strdup(candidate);

            return TRUE;
        }

        close(fd);
    }

    return FALSE;
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

    if (!port->running && port->thread)
        return;

    port->running = FALSE;

    if (port->thread) {
        g_thread_join(port->thread);
        port->thread = NULL;
    }
}

const gchar *uart_port_get_device_path(const UartPort *port) {
    if (!port)
        return NULL;

    return port->device_path;
}

gboolean uart_port_is_running(const UartPort *port) {
    if (!port)
        return FALSE;

    return port->running;
}