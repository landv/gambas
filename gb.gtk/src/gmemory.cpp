/***************************************************************************

  gmemory.cpp

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

#include <glib.h>
#include "main.h"
#include "gambas.h"
#include <stdio.h>

gpointer gMalloc (gsize n_bytes)
{
	gpointer *ptr;

	GB.Alloc((void**)POINTER(&ptr), n_bytes);
	return ptr;
}

gpointer gRealloc (gpointer mem,gsize n_bytes)
{
	GB.Realloc((void**)POINTER(&mem), n_bytes);
	return mem;
}

void gFree (gpointer mem)
{
	gpointer ptr = mem;
	GB.Free((void**)POINTER(&ptr));
}



void setGeneralMemoryHandler()
{
	GMemVTable general_memory_table;

	general_memory_table.malloc=gMalloc;
	general_memory_table.realloc=gRealloc;
	general_memory_table.free=gFree;
	general_memory_table.calloc=NULL;
	general_memory_table.try_malloc=NULL;
	general_memory_table.try_realloc=NULL;

	g_mem_set_vtable(&general_memory_table);
}


