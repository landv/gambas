#ifndef G_MEMORY_H
#define G_MEMORY_H

#include <glib.h>
#include "main.h"
#include "gambas.h"

gpointer gMalloc (gsize n_bytes);
gpointer gRealloc (gpointer mem,gsize n_bytes);
void gFree (gpointer mem);
void setGeneralMemoryHandler();

/*#define g_realloc(_p, _s) (g_print("g_realloc: %s.%d\n", __FILE__, __LINE__), gRealloc(_p, _s))
#define g_free(_p) (g_print("g_free: %s.%d\n", __FILE__, __LINE__), gFree(_p))
#define g_malloc(_s) (g_print("g_malloc: %s.%d\n", __FILE__, __LINE__), gMalloc(_s))*/

#endif
