/***************************************************************************

  gmemory.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

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
