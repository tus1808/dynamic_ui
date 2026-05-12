#ifndef COMMON_TIME_SETTER_H
#define COMMON_TIME_SETTER_H

#include <glib.h>

/* Sets the system clock to the given calendar time (local).
 * Requires CAP_SYS_TIME or root; logs a warning when it fails.
 * Returns TRUE on success. */
gboolean time_setter_apply(int year, int month, int day,
                           int hour, int minute, int second);

#endif
