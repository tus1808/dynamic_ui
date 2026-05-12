#include "net/ip_setter.h"

#include <glib.h>
#include <string.h>

#define DEFAULT_INTERFACE "eth0"
#define DEFAULT_PREFIX_LEN 24

gboolean ip_setter_apply(const char *ip) {
    gchar *cidr;
    gchar *stderr_buf = NULL;
    gint exit_status = 0;
    GError *error = NULL;
    gboolean ok;
    gchar *argv[8];

    if (!ip || ip[0] == '\0')
        return FALSE;

    /* refuse anything that already contains '/' (caller-supplied prefix not allowed in this minimal helper) */
    if (strchr(ip, '/')) {
        g_warning("[NET] ip_setter: refusing IP containing '/': %s", ip);
        return FALSE;
    }

    cidr = g_strdup_printf("%s/%d", ip, DEFAULT_PREFIX_LEN);

    argv[0] = (gchar *)"ip";
    argv[1] = (gchar *)"addr";
    argv[2] = (gchar *)"add";
    argv[3] = cidr;
    argv[4] = (gchar *)"dev";
    argv[5] = (gchar *)DEFAULT_INTERFACE;
    argv[6] = NULL;

    ok = g_spawn_sync(
        NULL,                  /* working dir */
        argv,
        NULL,                  /* envp */
        G_SPAWN_SEARCH_PATH,
        NULL, NULL,            /* child setup */
        NULL,                  /* stdout */
        &stderr_buf,           /* stderr */
        &exit_status,
        &error
    );

    if (!ok) {
        g_warning("[NET] could not run `ip`: %s",
                  error ? error->message : "unknown error");
        g_clear_error(&error);
        g_free(cidr);
        g_free(stderr_buf);
        return FALSE;
    }

    if (!g_spawn_check_exit_status(exit_status, &error)) {
        /* "RTNETLINK answers: File exists" is the common case when the address is already bound */
        const gchar *msg = stderr_buf && stderr_buf[0] ? stderr_buf : "non-zero exit";
        g_warning("[NET] could not set static IP %s on %s: %s",
                  cidr, DEFAULT_INTERFACE, msg);
        g_clear_error(&error);
        g_free(cidr);
        g_free(stderr_buf);
        return FALSE;
    }

    g_print("[NET] applied static IP %s on %s\n", cidr, DEFAULT_INTERFACE);
    g_free(cidr);
    g_free(stderr_buf);
    return TRUE;
}
