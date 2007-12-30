/***************************************************************************

  library.h

  GAMBAS plug-in management routines

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GBX_LIBRARY_H
#define __GBX_LIBRARY_H

#include "gambas.h"
#include "gbx_list.h"
#include "gb_component.h"

#include <dlfcn.h>

#ifndef DONT_USE_LTDL
  #include <ltdl.h>
#else
  #define lt_dlsym dlsym
  #define lt_dlclose dlclose
  #define lt_dlerror dlerror
#endif

typedef
  struct _LIBRARY {
#ifdef USE_LTDL
    lt_dlhandle handle;
#else
    void *handle;
#endif
    const char *name;
    void (*signal)();
    unsigned persistent : 1;
    unsigned _reserved: 13;
    }
  PACKED
  LIBRARY;

#ifndef __GBX_LIBRARY_C
#endif

PUBLIC void LIBRARY_preload(const char *file, char **argv);

PUBLIC void LIBRARY_init(void);
PUBLIC void LIBRARY_exit(void);

PUBLIC LIBRARY *LIBRARY_create(const char *path);
PUBLIC void LIBRARY_delete(LIBRARY *lib);

PUBLIC void LIBRARY_load(LIBRARY *lib);
PUBLIC void LIBRARY_unload(LIBRARY *lib);

PUBLIC void LIBRARY_declare(GB_DESC **desc);

PUBLIC boolean LIBRARY_get_interface_by_name(const char *name, long version, void *iface);
PUBLIC void LIBRARY_get_interface(LIBRARY *lib, long version, void *iface);

#endif
