#include "common/time_setter.h"

#include <glib.h>
#include <sys/time.h>
#include <time.h>

gboolean time_setter_apply(int year, int month, int day,
                           int hour, int minute, int second) {
    struct tm t = {0};
    struct timeval tv;
    time_t epoch;

    t.tm_year  = year - 1900;
    t.tm_mon   = month - 1;
    t.tm_mday  = day;
    t.tm_hour  = hour;
    t.tm_min   = minute;
    t.tm_sec   = second;
    t.tm_isdst = -1;

    epoch = mktime(&t);
    if (epoch == (time_t)-1) {
        g_warning("[TIME] invalid time values: %04d-%02d-%02d %02d:%02d:%02d",
                  year, month, day, hour, minute, second);
        return FALSE;
    }

    tv.tv_sec  = epoch;
    tv.tv_usec = 0;

    if (settimeofday(&tv, NULL) != 0) {
        g_warning("[TIME] failed to set system time (requires root or CAP_SYS_TIME)");
        return FALSE;
    }

    g_print("[TIME] System time set to: %04d-%02d-%02d %02d:%02d:%02d\n",
            year, month, day, hour, minute, second);
    return TRUE;
}
