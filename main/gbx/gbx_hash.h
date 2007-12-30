/***************************************************************************

  hash.h

  hash table routines, adapted from glib 1.2.8

  (c) 2000-2006 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GBX_HASH_H
#define __GBX_HASH_H

#include "gbx_compare.h"

#define KEEP_ORDER

typedef
  struct _HASH_NODE
  {
    struct _HASH_NODE *next;
    #ifdef KEEP_ORDER
    struct _HASH_NODE *snext;
    struct _HASH_NODE *sprev;
    #endif
  }
  HASH_NODE;

typedef
  struct
  {
    long size;
    long nnodes;
    HASH_NODE **nodes;
    size_t s_value;
    HASH_NODE *last;
    int mode;
    #ifdef KEEP_ORDER
    HASH_NODE *sfirst;
    HASH_NODE *slast;
    #endif
  }
  HASH_TABLE;

typedef
  struct
  {
    HASH_NODE *node;
    HASH_NODE *next;
  }
  HASH_ENUM;

typedef
  ulong (*HASH_FUNC)(const char *, long);

typedef
  bool (*HASH_COMP)(const char *, const char *, long);

#define HASH_TABLE_MIN_SIZE 11
#define HASH_TABLE_MAX_SIZE 13845163


PUBLIC void HASH_TABLE_create(HASH_TABLE **hash, size_t s_value, int mode);
PUBLIC void HASH_TABLE_delete(HASH_TABLE **hash);
PUBLIC long HASH_TABLE_size(HASH_TABLE *hash_table);
PUBLIC void *HASH_TABLE_lookup(HASH_TABLE *hash_table, const char *key, long len);
PUBLIC void *HASH_TABLE_insert(HASH_TABLE *hash_table, const char *key, long len);
PUBLIC void HASH_TABLE_remove(HASH_TABLE *hash_table, const char *key, long len);
PUBLIC void *HASH_TABLE_next(HASH_TABLE *hash_table, HASH_ENUM *iter);
PUBLIC void HASH_TABLE_get_key(HASH_TABLE *hash_table, HASH_NODE *node, char **key, long *len);
PUBLIC bool HASH_TABLE_get_last_key(HASH_TABLE *hash_table, char **key, long *len);

#endif

