/***************************************************************************

  gbx_component.h

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

#ifndef __GBX_COMPONENT_H
#define __GBX_COMPONENT_H

#include "gambas.h"
#include "gb_list.h"
#include "gb_component.h"

#include "gbx_library.h"
#include "gbx_archive.h"

typedef
  struct _COMPONENT {
    void *class;
    intptr_t ref;
    LIST list;
		LIST load;
    char *name;
    LIBRARY *library;
    ARCHIVE *archive;
		unsigned order : 8;
    unsigned loaded : 1;
    unsigned user : 1;       // user library
    unsigned warning : 1;    // Set when the bytecode warning was displayed by the class loader for this component
    unsigned loading : 1;    // component is being loaded
    unsigned _reserved : 20;
    }
  COMPONENT;

#ifndef __GBX_COMPONENT_C
EXTERN char *COMPONENT_path;
EXTERN COMPONENT *COMPONENT_current;
EXTERN int COMPONENT_count;
EXTERN COMPONENT *COMPONENT_main;
#endif

void COMPONENT_init(void);
void COMPONENT_exit(void);

COMPONENT *COMPONENT_create(const char *name);
void COMPONENT_delete(COMPONENT *comp);

COMPONENT *COMPONENT_find(const char *name);
bool COMPONENT_exist(const char *name);

void COMPONENT_load(COMPONENT *comp);
void COMPONENT_unload(COMPONENT *comp);

void COMPONENT_load_all(void);
void COMPONENT_load_all_finish(void);


COMPONENT *COMPONENT_next(COMPONENT *comp);

void COMPONENT_translation_must_be_reloaded(void);

void COMPONENT_signal(int signal, void *param);

#define COMPONENT_is_library(comp) ((comp)->library != NULL)

bool COMPONENT_get_info(const char *key, void **value);

void COMPONENT_exec(const char *name, int argc, char **argv);

#endif
