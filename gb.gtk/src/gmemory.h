#include <glib.h>
#include "main.h"
#include "gambas.h"

#ifndef G_MEMORY_H
#define G_MEMORY_H
gpointer gMalloc (gsize n_bytes);
gpointer gRealloc (gpointer mem,gsize n_bytes);
void gFree (gpointer mem);
void setGeneralMemoryHandler();
#endif
