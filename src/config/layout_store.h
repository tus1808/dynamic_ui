#ifndef LAYOUT_STORE_H
#define LAYOUT_STORE_H

#include <glib.h>
#include "common/types.h"

GPtrArray *layout_store_load(const char *file_path);
void layout_store_free_item(LayoutItem *item);

#endif