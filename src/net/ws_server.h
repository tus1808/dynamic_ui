#ifndef NET_WS_SERVER_H
#define NET_WS_SERVER_H

#include <glib.h>

#include "app/state.h"

/* Starts a libsoup WebSocket server bound on 0.0.0.0:<port>.
 *
 * Incoming text frames are parsed as JSON of one of these shapes:
 *   {"type":"data","values":[{"_id":"item 1","value":"42"}, ...]}
 *   {"type":"time","year":2026,"month":5,"day":11,"hour":10,"minute":30,"second":0}
 *
 * Returns TRUE on success. State is borrowed; caller owns AppState.
 */
gboolean ws_server_start(AppState *state, gint port);

/* Stops the server and releases libsoup resources. */
void ws_server_stop(void);

#endif
