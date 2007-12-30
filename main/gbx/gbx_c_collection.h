/***************************************************************************

  CCollection.c

  The Collection native class.

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

#ifndef __GBX_C_COLLECTION_H
#define __GBX_C_COLLECTION_H

#include "gambas.h"

#include "gb_hash.h"
#include "gbx_object.h"

typedef
  struct {
    long sort;
    void *data;
  }
  CCOL_DESC;

typedef
  struct {
    OBJECT object;
    HASH_TABLE *hash_table;
    HASH_NODE *last;
    int mode;
    }
  CCOLLECTION;

#ifndef __GBX_C_COLLECTION_C

extern GB_DESC NATIVE_Collection[];

#else

#define THIS ((CCOLLECTION *)_object)

#endif


/*PUBLIC void *CCOLLECTION_new(TYPE type);*/
/*
PUBLIC void *CCOLLECTION_get_key(CCOLLECTION *col, char *key, int len);
PUBLIC void *CCOLLECTION_add_key(CCOLLECTION *col, char *key, int len);
PUBLIC void CCOLLECTION_remove_key(CCOLLECTION *col, char *key, int len);
*/

#endif
