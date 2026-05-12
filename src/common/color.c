#include "common/color.h"

#include <stdio.h>
#include <string.h>

gboolean color_parse_hex(const char *hex, double *r, double *g, double *b, double *a) {
    unsigned int rr = 0, gg = 0, bb = 0, aa = 255;
    int matched;
    size_t len;

    if (r) *r = 0.0;
    if (g) *g = 0.0;
    if (b) *b = 0.0;
    if (a) *a = 1.0;

    if (!hex)
        return FALSE;

    if (hex[0] == '#')
        hex++;

    len = strlen(hex);
    if (len == 6) {
        matched = sscanf(hex, "%2x%2x%2x", &rr, &gg, &bb);
        if (matched != 3)
            return FALSE;
    } else if (len == 8) {
        matched = sscanf(hex, "%2x%2x%2x%2x", &rr, &gg, &bb, &aa);
        if (matched != 4)
            return FALSE;
    } else {
        return FALSE;
    }

    if (r) *r = rr / 255.0;
    if (g) *g = gg / 255.0;
    if (b) *b = bb / 255.0;
    if (a) *a = aa / 255.0;

    return TRUE;
}
