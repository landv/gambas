/***************************************************************************

  table.h

  Symbol tables management

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_TABLE_H
#define __GB_TABLE_H

#include "gb_array.h"

#define NO_SYMBOL (-1)

typedef
  struct {
    unsigned short sort;
    unsigned short len;
    char *name;
    }
  PACKED
  SYMBOL;

typedef
  enum {
    TF_NORMAL = 0,
    TF_IGNORE_CASE = 1
    }
  TABLE_FLAG;


typedef
  struct _table {
    SYMBOL *symbol;
    TABLE_FLAG flag;
    }
  TABLE;

PUBLIC void TABLE_create_static(TABLE *table, size_t size, TABLE_FLAG flag);
PUBLIC void TABLE_delete_static(TABLE *table);

PUBLIC void TABLE_create(TABLE **result, size_t size, TABLE_FLAG flag);
PUBLIC void TABLE_create_from(TABLE **result, size_t size, const char *sym_list[], TABLE_FLAG flag);
PUBLIC void TABLE_delete(TABLE **table);

PUBLIC int compare_ignore_case(const char *s1, long len1, const char *s2, long len2);

PUBLIC long TABLE_count(TABLE *table);
PUBLIC const char *TABLE_get_symbol_name(TABLE *table, long index);
PUBLIC const char *TABLE_get_symbol_name_suffix(TABLE *table, long index, const char* suffix);

PUBLIC bool TABLE_find_symbol(TABLE *table, const char *name, int len, SYMBOL **symbol, long *index);
PUBLIC bool TABLE_add_symbol(TABLE *table, const char *name, int len, SYMBOL **symbol, long *index);
PUBLIC void TABLE_sort(TABLE *table);
PUBLIC void TABLE_print(TABLE *table, boolean sort);
/*PUBLIC boolean TABLE_copy_symbol(TABLE *dst, TABLE *src, long index_src, SYMBOL **symbol, long *index);*/
PUBLIC void TABLE_add_new_symbol_without_sort(TABLE *table, const char *name, int len, long sort, SYMBOL **symbol, long *index);

PUBLIC bool SYMBOL_find(void *symbol, int n_symbol, size_t s_symbol, int flag, const char *name, int len, const char *prefix, long *result);

#define TABLE_get_symbol(table, ind) ((SYMBOL *)ARRAY_get((table)->symbol, ind))

PUBLIC SYMBOL *TABLE_get_symbol_sort(TABLE *table, long index);
PUBLIC void TABLE_copy_symbol_with_prefix(TABLE *table, long ind_src, char prefix, SYMBOL **symbol, long *index);

#endif
