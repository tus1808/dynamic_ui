#pragma once

#include <gtk/gtk.h>

#include "common/types.h"

/* Returns a GtkDrawingArea that ticks every second.
 * `is_date` toggles whether `mm` in the format string maps to minutes (FALSE)
 * or months (TRUE). Returns NULL when cfg->visibility is FALSE. */
GtkWidget *ui_clock_create(const ClockConfig *cfg, gboolean is_date);
