/***************************************************************************

  gbx_c_collection.h

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

#ifndef __GBX_C_COLLECTION_H
#define __GBX_C_COLLECTION_H

#include "gambas.h"

#include "gb_hash.h"
#include "gbx_object.h"

typedef
  struct {
    int sort;
    void *data;
  }
  CCOL_DESC;

typedef
  struct {
    OBJECT object;
    HASH_TABLE *hash_table;
    HASH_NODE *last;
    short mode;
		char locked;
    }
  CCOLLECTION;

#ifndef __GBX_C_COLLECTION_C

extern GB_DESC NATIVE_Collection[];

#else

#define THIS ((CCOLLECTION *)_object)

#endif

#define CCOLLECTION_get_count(_col) HASH_TABLE_size((_col)->hash_table)

/*PUBLIC void *CCOLLECTION_new(TYPE type);*/
/*
void *CCOLLECTION_get_key(CCOLLECTION *col, char *key, int len);
void *CCOLLECTION_add_key(CCOLLECTION *col, char *key, int len);
void CCOLLECTION_remove_key(CCOLLECTION *col, char *key, int len);
*/

#endif
