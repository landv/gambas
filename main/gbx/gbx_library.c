/***************************************************************************

  gbx_library.c

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

#define __GBX_LIBRARY_C

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_replace.h"
#include "gb_magic.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#include "gb_error.h"

#include "gbx_class.h"
#include "gbx_exec.h"
#include "gbx_event.h"
#include "gbx_stack.h"
#include "gb_file.h"
#include "gbx_archive.h"
#include "gbx_project.h"
#include "gbx_api.h"

#include "gbx_string.h"
#include "gbx_object.h"
#include "gbx_component.h"

#include "gbx_library.h"

//#define DEBUG
//#define DEBUG_PRELOAD

// Maximum size of a project or startup file
// This avoids reading too much in the archive file!

#define MAX_SIZE 2048

int _bytes_read = 0;

#if 0
static lt_ptr library_malloc(size_t size)
{
  lt_ptr ptr;
  ALLOC(&ptr, size, "library_malloc");
  printf("library_malloc: %d -> %p\n", size, ptr);
  return ptr;
}


static void library_free(lt_ptr ptr)
{
  printf("library_free -> %p\n", ptr);
  FREE(&ptr, "library_free");
}
#endif

static void *get_symbol(LIBRARY *lib, const char *symbol, bool err)
{
  void *sym;

  sym = lt_dlsym(lib->handle, symbol);
  if (sym == NULL && err)
  {
    strcpy(COMMON_buffer, lt_dlerror());
    lt_dlclose(lib->handle);
    lib->handle = NULL;
    THROW(E_LIBRARY, lib->name, COMMON_buffer);
  }

  return sym;
}


static void copy_interface(void **src, void **dst)
{
  for(;;)
  {
    if (!*src)
      return;

    *dst++ = *src++;
  }
}


void LIBRARY_init(void)
{
  /*if (putenv("LD_BIND_NOW=true"))
    ERROR_panic("Cannot set LD_BIND_NOW: &1", strerror(errno));

  if (putenv("KDE_MALLOC=0"))
    ERROR_panic("Cannot set KDE_MALLOC: &1", strerror(errno));*/

  /*lt_dlmalloc = library_malloc;
  lt_dlfree = library_free;*/

  #ifndef DONT_USE_LTDL
  if (lt_dlinit())
    ERROR_panic("Cannot initialize plug-in management: %s", lt_dlerror());
  #endif
}


void LIBRARY_exit(void)
{
  #ifndef DONT_USE_LTDL
  lt_dlexit();
  #endif
}


void LIBRARY_get_interface(LIBRARY *lib, int version, void *iface)
{
  char symbol[32];
  int i, len;
  char c;

  len = strlen(lib->name);
  for (i = 0; i < len; i++)
  {
    c = toupper(lib->name[i]);
    if (!isalnum((unsigned char)c))
      c = '_';

    symbol[i] = c;
  }

  sprintf(&symbol[len], "_%d", version);
  
  copy_interface((void **)get_symbol(lib, symbol, TRUE), iface);
}


bool LIBRARY_get_interface_by_name(const char *name, int version, void *iface)
{
  COMPONENT *comp;

  comp = COMPONENT_find(name);
  if (!comp || !comp->library)
    return TRUE;

  LIBRARY_get_interface(comp->library, version, iface);
  return FALSE;
}



LIBRARY *LIBRARY_create(const char *name)
{
  LIBRARY *lib;

  ALLOC_ZERO(&lib, sizeof(LIBRARY));

  lib->handle = NULL;
  lib->name = name;

  /*if (name)
  {
    lib->persistent = FALSE;
    lib->preload = FALSE;
  }
  else
  {
    lib->persistent = TRUE;
    lib->preload = TRUE;
  }*/

  return lib;
}


void LIBRARY_delete(LIBRARY *lib)
{
  LIBRARY_unload(lib);
  FREE(&lib);
}


int LIBRARY_load(LIBRARY *lib)
{
  int (*func)();
  void **iface;
  GB_DESC **desc;
  char *path;
	int order = 0;

  if (lib->handle)
    return 0;

#ifdef DEBUG
  clock_t t = clock();
  fprintf(stderr, "Loading library %s\n", lib->name);
#endif

  path = FILE_buffer();
  sprintf(path, LIB_PATTERN, COMPONENT_path, lib->name);

  //if (!FILE_exist(path))
	//  sprintf(path, LIB_PATTERN, COMPONENT_user_path, lib->name);

  #ifndef DONT_USE_LTDL
    /* no more available in libltld ?
    lt_dlopen_flag = RTLD_LAZY;
    */
    lib->handle = lt_dlopenext(path);
  #else
    lib->handle = dlopen(path, RTLD_LAZY);
  #endif

  if (lib->handle == NULL)
    THROW(E_LIBRARY, lib->name, lt_dlerror());

  func = get_symbol(lib, LIB_INIT, TRUE);

  /* Interface de Gambas */

  iface = get_symbol(lib, LIB_GAMBAS_PTR, FALSE);
	if (iface)
		*((void **)iface) = &GAMBAS_Api;
	else
	{
		iface = get_symbol(lib, LIB_GAMBAS, TRUE);
		copy_interface(GAMBAS_Api, iface);
	}

	/* Signal function */
	lib->signal = (void(*)())get_symbol(lib, LIB_SIGNAL, FALSE);
	lib->info = (int(*)())get_symbol(lib, LIB_INFO, FALSE);

  /* Initialisation */
  order = (*func)();
	
  /* Déclaration des classes */
  desc = get_symbol(lib, LIB_CLASS, FALSE);
  if (desc)
    LIBRARY_declare(desc);

#ifdef DEBUG
  fprintf(stderr, "Library %s loaded ", lib->name);
  fprintf(stderr, "in %g s\n", ((double)(clock() - t) / CLOCKS_PER_SEC));
#endif

	return order;
}

void LIBRARY_exec(LIBRARY *lib, int argc, char **argv)
{
  void (*func)();
	
  func = get_symbol(lib, LIB_MAIN, FALSE);
	if (func)
		(*func)(argc, argv);
}


void LIBRARY_declare(GB_DESC **desc)
{
  GB_DESC **p;

  p = desc;
  while (*p != NULL)
  {
    CLASS_find_global((*p)->name);
    p++;
  }

  p = desc;
  while (*p != NULL)
  {
    if (CLASS_register(*p) == NULL)
      THROW(E_REGISTER, (*p)->name);

    p++;
  }
}


void LIBRARY_unload(LIBRARY *lib)
{
  void (*gambas_exit)();

  if (lib->handle == NULL)
    return;

  /* Pas de lib�ation des classes pr�harg� ! */

  /* V�ification qu'aucune classe de la librairie n'est instanci� ! */

  gambas_exit = lt_dlsym(lib->handle, LIB_EXIT);
  if (gambas_exit != NULL)
    (*gambas_exit)();

  if (lib->persistent)
  {
    gambas_exit = lt_dlsym(lib->handle, "_fini");
    if (gambas_exit != NULL)
      (*gambas_exit)();
  }
  else
	{
    lt_dlclose(lib->handle);
	}

  lib->handle = NULL;

#ifdef DEBUG
  printf("Unloading library %s\n", lib->name);
#endif
}

