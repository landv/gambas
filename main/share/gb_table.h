/***************************************************************************

	gb_table.h

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __GB_TABLE_H
#define __GB_TABLE_H

#include "gb_array.h"

#define NO_SYMBOL (-1)
//#define TABLE_USE_KEY 1

typedef
	struct {
		char *name;
		int len;
#if TABLE_USE_KEY
		uint key;
#endif
		}
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
		ushort *sort;
		TABLE_FLAG flag;
		}
	TABLE;

#ifndef __GB_TABLE_C
extern bool TABLE_new_symbol;
#endif
	
void TABLE_create_static(TABLE *table, size_t size, TABLE_FLAG flag);
void TABLE_delete_static(TABLE *table);

void TABLE_create(TABLE **result, size_t size, TABLE_FLAG flag);
void TABLE_create_from(TABLE **result, size_t size, const char *sym_list[], TABLE_FLAG flag);
void TABLE_delete(TABLE **table);

char TABLE_compare_ignore_case(const char *s1, int len1, const char *s2, int len2);
char TABLE_compare(const char *s1, int len1, const char *s2, int len2);
char TABLE_compare_ignore_case_len(const char *s1, int len1, const char *s2, int len2);

#define TABLE_count(_table) (ARRAY_count((_table)->symbol))
const char *TABLE_get_symbol_name(TABLE *table, int index);
const char *TABLE_get_symbol_name_suffix(TABLE *table, int index, const char* suffix);
const char *SYMBOL_get_name(SYMBOL *sym);

bool TABLE_find_symbol(TABLE *table, const char *name, int len, int *index);
int TABLE_add_symbol(TABLE *table, const char *name, int len);

#define TABLE_add_symbol_exist(_table, _name, _len, _pindex) \
({ \
	TABLE_new_symbol = FALSE; \
	*_pindex = TABLE_add_symbol((_table), (_name), (_len)); \
	!TABLE_new_symbol; \
})
	

void TABLE_print(TABLE *table, bool sort);

#define TABLE_get_symbol(table, ind) ((SYMBOL *)ARRAY_get((table)->symbol, ind))

SYMBOL *TABLE_get_symbol_sort(TABLE *table, int index);
int TABLE_copy_symbol_with_prefix(TABLE *table, int ind_src, char prefix);

int SYMBOL_find(void *symbol, ushort *sort, int n_symbol, size_t s_symbol, int flag, const char *name, int len, const char *prefix);
#if TABLE_USE_KEY
void SYMBOL_compute_keys(void *symbol, int n_symbol, size_t s_symbol, int flag);
#else
#define SYMBOL_compute_keys(_symbol, _nsymbol, _ssymbol, _flag)
#endif
#endif

#define SYMBOL_compare(_sym, _name) TABLE_compare((_sym)->name, (_sym)->len, (_name), strlen(_name))
#define SYMBOL_compare_ignore_case(_sym, _name) TABLE_compare_ignore_case((_sym)->name, (_sym)->len, (_name), strlen(_name))
