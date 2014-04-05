/***************************************************************************

  gb_hash.h

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

#ifndef __GB_HASH_H
#define __GB_HASH_H

#define KEEP_ORDER

typedef
  enum {
    HF_NORMAL = 0,
    HF_IGNORE_CASE = 1
    }
  HASH_FLAG;

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
  struct {
    ushort len;
    char key[0];
    }
  PACKED
  HASH_KEY;

typedef
  struct
  {
    int size;
    int nnodes;
    HASH_NODE **nodes;
    size_t s_value;
    HASH_NODE *last;
    HASH_FLAG mode;
    #ifdef KEEP_ORDER
    HASH_NODE *sfirst;
    HASH_NODE *slast;
    #endif
  }
  HASH_TABLE;

// NOTE: If HASH_ENUM changes, GB_COLLECTION_ITER must be updated accordingly in gambas.h
	
typedef
  struct
  {
    HASH_NODE *node;
    HASH_NODE *next;
  }
  HASH_ENUM;

typedef
  uint (*HASH_FUNC)(const char *, int);

typedef
  bool (*HASH_COMP)(const char *, const char *, int);

#define HASH_TABLE_MIN_SIZE 11
#define HASH_TABLE_MAX_SIZE 13845163

#ifndef __GB_HASH_C
extern uint HASH_seed;
#endif
	
void HASH_TABLE_create(HASH_TABLE **hash, size_t s_value, HASH_FLAG mode);
void HASH_TABLE_delete(HASH_TABLE **hash);
int HASH_TABLE_size(HASH_TABLE *hash_table);
void *HASH_TABLE_lookup(HASH_TABLE *hash_table, const char *key, int len, bool set_last);
void *HASH_TABLE_insert(HASH_TABLE *hash_table, const char *key, int len);
void HASH_TABLE_remove(HASH_TABLE *hash_table, const char *key, int len);
void *HASH_TABLE_next(HASH_TABLE *hash_table, HASH_ENUM *iter, bool set_last);
void HASH_TABLE_get_key(HASH_TABLE *hash_table, HASH_NODE *node, char **key, int *len);
bool HASH_TABLE_get_last_key(HASH_TABLE *hash_table, char **key, int *len);
void HASH_TABLE_set_last_key(HASH_TABLE *hash_table, char *key, int len);

#endif

