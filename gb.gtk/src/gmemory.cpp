#include <glib.h>
#include "main.h"
#include "gambas.h"
#include <stdio.h>

gpointer gMalloc (gsize n_bytes)
{
	gpointer *ptr;

	GB.Alloc ((void**)&ptr,n_bytes);
	return ptr;
}

gpointer gRealloc (gpointer mem,gsize n_bytes)
{
	GB.Realloc ((void**)&mem,n_bytes);
	return mem;
}

void gFree (gpointer mem)
{
	gpointer ptr=mem;
	GB.Free((void**)&ptr);
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


