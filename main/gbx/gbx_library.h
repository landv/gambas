/***************************************************************************

  gbx_library.h

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

#ifndef __GBX_LIBRARY_H
#define __GBX_LIBRARY_H

#include "gambas.h"
#include "gb_list.h"
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
    int (*info)();
    unsigned persistent : 1;
    unsigned _reserved: 13;
    }
  LIBRARY;

#ifndef __GBX_LIBRARY_C
#endif

void LIBRARY_init(void);
void LIBRARY_exit(void);

LIBRARY *LIBRARY_create(const char *path);
void LIBRARY_delete(LIBRARY *lib);

int LIBRARY_load(LIBRARY *lib);
void LIBRARY_unload(LIBRARY *lib);

void LIBRARY_declare(GB_DESC **desc);

bool LIBRARY_get_interface_by_name(const char *name, int version, void *iface);
void LIBRARY_get_interface(LIBRARY *lib, int version, void *iface);

void LIBRARY_exec(LIBRARY *lib, int argc, char **argv);

#endif
