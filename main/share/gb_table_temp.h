/***************************************************************************

  gb_table_temp.h

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

#define __GB_TABLE_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_alloc.h"
#include "gb_limit.h"

#include "gb_table.h"

#define SYM(table, ind) (TABLE_get_symbol(table, ind))
#define SSYM(_symbol, _pos, _size) ((SYMBOL *)((char *)(_symbol) + (_pos) * (_size)))

bool TABLE_new_symbol = FALSE;
static char _buffer[MAX_SYMBOL_LEN + 1];

#if TABLE_USE_KEY
static inline uint make_key(const char *sym, int len)
{
	static void *jump[] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3 };
	const uchar *p = (uchar *)sym;
	
	if (len >= 4)
		return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	
	goto *jump[len];
	
	__LEN_0: return 0;
	__LEN_1: return (p[0] << 24);
	__LEN_2: return (p[0] << 24) + (p[1] << 16);
	__LEN_3: return (p[0] << 24) + (p[1] << 16) + (p[2] << 8);
}

static inline uint make_key_ignore_case(const char *sym, int len)
{
	static void *jump[] = { &&__LEN_0, &&__LEN_1, &&__LEN_2, &&__LEN_3 };
	const uchar *p = (uchar *)sym;
	
	if (len >= 4)
		return (tolower(p[0]) << 24) + (tolower(p[1]) << 16) + (tolower(p[2]) << 8) + tolower(p[3]);
	
	goto *jump[len];
	
	__LEN_0: return 0;
	__LEN_1: return (tolower(p[0]) << 24);
	__LEN_2: return (tolower(p[0]) << 24) + (tolower(p[1]) << 16);
	__LEN_3: return (tolower(p[0]) << 24) + (tolower(p[1]) << 16) + (tolower(p[2]) << 8);
}
#endif

char TABLE_compare(const char *s1, int len1, const char *s2, int len2)
{
	int i;
	int len = (len1 < len2) ? len1 : len2;
	register unsigned char c1;
	register unsigned char c2;

	for (i = 0; i < len; i++)
	{
		c1 = s1[i];
		c2 = s2[i];

		if (LIKELY(c1 > c2)) return 1;
		if (LIKELY(c1 < c2)) return -1;
	}

	if (LIKELY(len1 < len2))
		return -1;
	else if (LIKELY(len1 > len2))
		return 1;
	else
		return 0;
}

char TABLE_compare_ignore_case(const char *s1, int len1, const char *s2, int len2)
{
	unsigned int len = (len1 < len2) ? len1 : len2;
	unsigned int i;
	int result;

	for (i = 0; len > 0; i++)
	{
		result = toupper(s1[i]) - toupper(s2[i]);
		if (LIKELY(result))
			return result; // < 0 ? -1 : 1;
		len--;
	}

	if (LIKELY(len1 < len2))
		return -1;
	else if (LIKELY(len1 > len2))
		return 1;
	else
		return 0;
}

char TABLE_compare_ignore_case_len(const char *s1, int len1, const char *s2, int len2)
{
	int result;

	if (LIKELY(len1 < len2))
		return -1;
	else if (LIKELY(len1 > len2))
		return 1;

	while (len1)
	{
		result = tolower(*s1++) - tolower(*s2++);
		if (LIKELY(result))
			return result; // < 0 ? -1 : 1;
		--len1;
	}

	return 0;
}

static inline int search(void *symbol, ushort *sort, int n_symbol, size_t size, const char *name, int len)
{
	int pos, deb, fin;
	SYMBOL *sym;
	int l;
	int result; // must be an integer (or a short) because uchar - uchar may not fit in a char!
	const uchar *s1;
	const uchar *s2;
#if TABLE_USE_KEY
	uint key = make_key(name, len);
#endif
	
	pos = 0;
	deb = 0;
	fin = n_symbol; //ARRAY_count(table->symbol);

	for(;;)
	{
		if (UNLIKELY(deb >= fin))
			return (-deb) - 1;

		pos = (deb + fin) >> 1;

		sym = SSYM(symbol, sort[pos], size);
		
		if (LIKELY(len < sym->len))
			goto __B_LOWER;
		else if (LIKELY(len > sym->len))
			goto __B_GREATER;
		
#if TABLE_USE_KEY
		if (key < sym->key) goto __B_LOWER;
		if (key > sym->key) goto __B_GREATER;
		
		l = len - 4;
		
		if (l > 0)
		{
			s1 = (uchar *)name + 4;
			s2 = (uchar *)sym->name + 4;
			
#else
		{
			l = len;
			s1 = (uchar *)name;
			s2 = (uchar *)sym->name;
#endif
			for(;;)
			{
				result = *s1 - *s2;
				
				if (LIKELY(result < 0))
					goto __B_LOWER;
				else if (LIKELY(result > 0))
					goto __B_GREATER;
				
				if (UNLIKELY(--l == 0))
					break;
				
				s1++;
				s2++;
			}
		}

		return pos;
		
		__B_LOWER: fin = pos; continue;
		__B_GREATER: deb = pos + 1; continue;
	}    
}


static inline int search_ignore_case(void *symbol, ushort *sort, int n_symbol, size_t size, const char *name, int len)
{
	int pos, deb, fin;
	SYMBOL *sym;
	int l;
	int result; // must be an integer (or a short) because uchar - uchar may not fit in a char!
	const uchar *s1;
	const uchar *s2;
#if TABLE_USE_KEY
	uint key = make_key_ignore_case(name, len);
#endif

	pos = 0;
	deb = 0;
	fin = n_symbol;

	for(;;)
	{
		if (UNLIKELY(deb >= fin))
		{
			return -deb - 1;
			/**index = deb;
			return FALSE;*/
		}

		pos = (deb + fin) >> 1;

		sym = SSYM(symbol, sort[pos], size);

		if (LIKELY(len < sym->len))
			goto __T_LOWER;
		else if (LIKELY(len > sym->len))
			goto __T_GREATER;

#if TABLE_USE_KEY
		if (key < sym->key) goto __T_LOWER;
		if (key > sym->key) goto __T_GREATER;
		
		l = len - 4;
		
		if (l > 0)
		{
			s1 = (uchar *)name + 4;
			s2 = (uchar *)sym->name + 4;
#else
		{
			l = len;
			s1 = (uchar *)name;
			s2 = (uchar *)sym->name;
#endif
		
			for(;;)
			{
				result = tolower(*s1) - tolower(*s2);
				
				if (LIKELY(result < 0))
					goto __T_LOWER;
				else if (LIKELY(result > 0))
					goto __T_GREATER;
				
				if (UNLIKELY(--l == 0))
					break;
				
				s1++;
				s2++;
			}
		}

		return pos;
		/**index = pos;
		return TRUE;*/
		
		__T_LOWER: fin = pos; continue;
		__T_GREATER: deb = pos + 1; continue;
	}
}


const char *TABLE_get_symbol_name(TABLE *table, int index)
{
	if (UNLIKELY((index < 0) || (index >= ARRAY_count(table->symbol))))
		strcpy(_buffer, "?");
	else
		SYMBOL_get_name(SYM(table, index));

	return _buffer;
}


const char *TABLE_get_symbol_name_suffix(TABLE *table, int index, const char* suffix)
{
	SYMBOL *sym;
	
	if (UNLIKELY((index < 0) || (index >= ARRAY_count(table->symbol))))
		return "?";
	
	sym = SYM(table, index);
	if ((sym->len + strlen(suffix)) > MAX_SYMBOL_LEN)
		return "?";
	
	SYMBOL_get_name(sym);
	strcat(_buffer, suffix);
	return _buffer;
}


void TABLE_create_static(TABLE *table, size_t size, TABLE_FLAG flag)
{
	ARRAY_create_with_size(&table->symbol, Max(size, sizeof(SYMBOL)), 64);
	ARRAY_create_with_size(&table->sort, sizeof(ushort), 64);
	table->flag = flag;
}


void TABLE_create(TABLE **result, size_t size, TABLE_FLAG flag)
{
	TABLE *table;

	ALLOC(&table, sizeof(TABLE));
	TABLE_create_static(table, size, flag);

	*result = table;
}


void TABLE_create_from(TABLE **result, size_t size, const char *sym_list[], TABLE_FLAG flag)
{
	TABLE *table;

	TABLE_create(&table, size, flag);

	while (*sym_list)
	{
		TABLE_add_symbol(table, *sym_list, strlen(*sym_list));
		sym_list++;
	}

	*result = table;
}


void TABLE_delete_static(TABLE *table)
{
	ARRAY_delete(&table->symbol);
	ARRAY_delete(&table->sort);
}


void TABLE_delete(TABLE **p_table)
{
	if (*p_table)
	{
		TABLE_delete_static(*p_table);
		FREE(p_table);
	}
}


bool TABLE_find_symbol(TABLE *table, const char *name, int len, int *index)
{
	int ind;
	SYMBOL *tsym;
	int count;
	size_t size;

	tsym = table->symbol;
	count = ARRAY_count(tsym);
	size = ARRAY_size(tsym);

	if (table->flag)
		ind = search_ignore_case(tsym, table->sort, count, size, name, len);
	else
		ind = search(tsym, table->sort, count, size, name, len);
	
	if (ind >= 0)
	{
		*index = table->sort[ind];
		return TRUE;
	}
	else
		return FALSE;
}

#if 0
void TABLE_add_new_symbol_without_sort(TABLE *table, const char *name, int len, int sort, SYMBOL **symbol, int *index)
{
	SYMBOL *sym;
	int count;

	len = Min(len, MAX_SYMBOL_LEN);

	count = ARRAY_count(table->symbol);

	sym = (SYMBOL *)ARRAY_add_void_size(&table->symbol);

	sym->name = (char *)name;
	sym->len = len;
	
	*((ushort *)ARRAY_add(&table->sort)) = sort;

	if (symbol) *symbol = sym; /*&table->symbol[ind];*/
	if (index) *index = count;
}
#endif

int TABLE_add_symbol(TABLE *table, const char *name, int len)
{
	int ind;
	SYMBOL *sym;
	int count;
	size_t size;

	/*len = Min(len, MAX_SYMBOL_LEN);*/
	//len = Min(len, 65535);

	count = ARRAY_count(table->symbol);
	size = ARRAY_size(table->symbol);
	
	if (table->flag)
		ind = search_ignore_case(table->symbol, table->sort, count, size, name, len);
	else
		ind = search(table->symbol, table->sort, count, size, name, len);

	if (ind < 0)
	{
		ind = -ind - 1;
		sym = (SYMBOL *)ARRAY_add_void_size(&table->symbol);

		sym->name = (char *)name;
		sym->len = len;
#if TABLE_USE_KEY
		sym->key = table->flag ? make_key_ignore_case(name, len) : make_key(name, len);
#endif
		
		/*
		printf("TABLE_add_symbol: %.*s %d %d\n", len, name, ((CLASS_SYMBOL *)sym)->global.type,
			((CLASS_SYMBOL *)sym)->local.type);
		*/
		
		//s1 = (SYMBOL *)((char *)table->symbol + count * size);
		ARRAY_add(&table->sort);
		if (count > ind)
			memmove(&table->sort[ind + 1], &table->sort[ind], sizeof(ushort) * (count - ind));

		table->sort[ind] = (ushort)count;
		ind = count;
		TABLE_new_symbol = TRUE;
	}
	else
		ind = table->sort[ind]; //SYM(table, ind)->sort; /*table->symbol[ind].sort;*/

	return ind;
}

void TABLE_print(TABLE *table, bool sort)
{
	int i;
	SYMBOL *sym;

	fprintf(stderr, "capacity %i\n", ARRAY_count(table->symbol));

	/*
	for (i = 0; i < ARRAY_count(table->symbol); i++)
	{
		sym = SYM(table, i);
		printf("%*s (%li) ", (int)sym->len, sym->name, sym->sort);
	}

	printf("\n");
	*/

	for (i = 0; i < ARRAY_count(table->symbol); i++)
	{
		if (sort)
		{
			sym = SYM(table, table->sort[i]);
			fprintf(stderr, "%.*s ", (int)sym->len, sym->name);
		}
		else
		{
			sym = SYM(table, i);
			fprintf(stderr, "%d %.*s ", (int)table->sort[i], (int)sym->len, sym->name);
		}

		//if ((i > 0) && (!(i & 0xF)))
		//  fprintf(stderr, "\n");
	}

	fprintf(stderr, "\n\n");
}


int TABLE_copy_symbol_with_prefix(TABLE *table, int ind_src, char prefix)
{
	SYMBOL *sym;
	char *ptr;

	sym = TABLE_get_symbol(table, ind_src);

	ptr = (char *)sym->name - 1;

	if (UNLIKELY(!isspace((unsigned char)*ptr)))
		ERROR_panic("Cannot add prefix to symbol");

	*ptr = prefix;

	return TABLE_add_symbol(table, ptr, sym->len + 1);
}


SYMBOL *TABLE_get_symbol_sort(TABLE *table, int index)
{
	return TABLE_get_symbol(table, table->sort[index]);
}

// Symbol tables with no TABLE structure

int SYMBOL_find(void *symbol, ushort *sort, int n_symbol, size_t s_symbol, int flag, const char *name, int len, const char *prefix)
{
	int index;
	int len_prefix;

	if (UNLIKELY(prefix != NULL))
	{
		len_prefix = strlen(prefix);

		if (UNLIKELY((len + len_prefix) > MAX_SYMBOL_LEN))
			ERROR_panic("SYMBOL_find: prefixed symbol too long");

		strcpy(_buffer, prefix);
		strcpy(&_buffer[len_prefix], name);
		len += len_prefix;
		name = _buffer;
	}

	if (flag)
		index = search_ignore_case(symbol, sort, n_symbol, s_symbol, name, len);
	else
		index = search(symbol, sort, n_symbol, s_symbol, name, len);

	if (index >= 0)
		return sort[index];
	else
		return NO_SYMBOL;
}


const char *SYMBOL_get_name(SYMBOL *sym)
{
	int len;

	len = Min(MAX_SYMBOL_LEN, sym->len);
	memcpy(_buffer, sym->name, len);
	_buffer[len] = 0;

	return _buffer;
}

#if TABLE_USE_KEY
void SYMBOL_compute_keys(void *symbol, int n_symbol, size_t s_symbol, int flag)
{
	int i;
	SYMBOL *sym;
	
	if (flag)
	{
		for (i = 0; i < n_symbol; i++)
		{
			sym = SSYM(symbol, i, s_symbol);
			sym->key = make_key_ignore_case(sym->name, sym->len);
		}
	}
	else
	{
		for (i = 0; i < n_symbol; i++)
		{
			sym = SSYM(symbol, i, s_symbol);
			sym->key = make_key(sym->name, sym->len);
		}
	}
}
#endif
