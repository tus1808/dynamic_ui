#ifndef NET_IP_SETTER_H
#define NET_IP_SETTER_H

#include <glib.h>

/* Tries to add the given IPv4 address (without prefix) as a /24 on eth0
 * by spawning `ip addr add <ip>/24 dev eth0`.
 *
 * Returns TRUE on success. Non-fatal on failure (logs a warning) so the app
 * keeps booting even if it cannot adjust networking (e.g. no root, no eth0). */
gboolean ip_setter_apply(const char *ip);

#endif
