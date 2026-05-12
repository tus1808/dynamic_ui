#ifndef COMMON_COLOR_H
#define COMMON_COLOR_H

#include <glib.h>

/* Parses "#RRGGBB" or "#RRGGBBAA" hex strings into r/g/b/a in 0..1.
 * Returns TRUE on success. Outputs are set to 0/0/0/1 on failure (opaque black).
 */
gboolean color_parse_hex(const char *hex, double *r, double *g, double *b, double *a);

#endif
