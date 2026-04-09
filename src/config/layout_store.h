#ifndef LAYOUT_STORE_H
#define LAYOUT_STORE_H

#include <glib.h>
#include "common/types.h"

GPtrArray *layout_store_load(const char *file_path);
gboolean layout_store_save(const char *file_path, GPtrArray *items);
void layout_store_free_item(LayoutItem *item);

#endif