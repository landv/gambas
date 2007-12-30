/***************************************************************************

  component.h

  GAMBAS component management routines

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

#ifndef __GBX_COMPONENT_H
#define __GBX_COMPONENT_H

#include "gambas.h"
#include "gbx_list.h"
#include "gb_component.h"

#include "gbx_library.h"
#include "gbx_archive.h"

typedef
  struct _COMPONENT {
    void *class;
    long ref;
    LIST list;
    char *name;
    LIBRARY *library;
    ARCHIVE *archive;
    unsigned preload : 1;
    unsigned loaded : 1;
    unsigned user : 1;
    unsigned _reserved : 29;
    }
  PACKED
  COMPONENT;

#ifndef __GBX_COMPONENT_C
EXTERN char *COMPONENT_path;
EXTERN char *COMPONENT_user_path;
EXTERN COMPONENT *COMPONENT_current;
EXTERN int COMPONENT_count;
EXTERN COMPONENT *COMPONENT_main;
#endif

PUBLIC void COMPONENT_init(void);
PUBLIC void COMPONENT_exit(void);

PUBLIC COMPONENT *COMPONENT_create(const char *name);
PUBLIC void COMPONENT_delete(COMPONENT *comp);

PUBLIC COMPONENT *COMPONENT_find(const char *name);

PUBLIC void COMPONENT_load(COMPONENT *comp);
PUBLIC void COMPONENT_load_all(void);
PUBLIC void COMPONENT_unload(COMPONENT *comp);

PUBLIC COMPONENT *COMPONENT_next(COMPONENT *comp);

PUBLIC void COMPONENT_translation_must_be_reloaded(void);

PUBLIC void COMPONENT_signal(int signal, void *param);

#define COMPONENT_is_library(comp) ((comp)->library != NULL)

PUBLIC bool COMPONENT_get_info(const char *key, void **value);

#endif
