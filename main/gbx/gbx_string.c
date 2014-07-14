/***************************************************************************

  gbx_string.c

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

#define __STRING_C

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"
#include "gb_common_string.h"

#include "gb_error.h"
#include "gbx_value.h"
#include "gbx_debug.h"
#include "gbx_local.h"
#include "gbx_project.h"
#include "gbx_exec.h"

#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#include <iconv.h>

#include "gbx_string.h"

//#define DEBUG_ME

#if DEBUG_STRING
#define DEBUG_ME
#endif

#if DEBUG_STRING
char *STRING_watch = NULL;
#endif

/*#ifdef DEBUG_ME
extern FILE *MEMORY_log;
#undef stderr
#define stderr MEMORY_log
static void print_where()
{
	fprintf(MEMORY_log, "%s: ", DEBUG_get_current_position());
}
#define DEBUG_where print_where
#endif*/

#if DEBUG_MEMORY

static void *_my_malloc(int size)
{
	void *ptr;
	ALLOC(&ptr, size);
	return ptr;
}

static void _my_free(void *ptr)
{
	IFREE(ptr);
}

static void *_my_realloc(void *ptr, int size)
{
	REALLOC(&ptr, size);
	return ptr;
}
#else

#define _my_malloc my_malloc
#define _my_free my_free
#define _my_realloc my_realloc

#endif

#define STRING_last_count 32
static char *STRING_last[STRING_last_count] = { 0 };

const char STRING_char_string[512] = 
"\x00\x00\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00\x09\x00\x0A\x00\x0B\x00\x0C\x00\x0D\x00\x0E\x00\x0F\x00"
"\x10\x00\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00\x16\x00\x17\x00\x18\x00\x19\x00\x1A\x00\x1B\x00\x1C\x00\x1D\x00\x1E\x00\x1F\x00"
"\x20\x00\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00\x26\x00\x27\x00\x28\x00\x29\x00\x2A\x00\x2B\x00\x2C\x00\x2D\x00\x2E\x00\x2F\x00"
"\x30\x00\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00\x36\x00\x37\x00\x38\x00\x39\x00\x3A\x00\x3B\x00\x3C\x00\x3D\x00\x3E\x00\x3F\x00"
"\x40\x00\x41\x00\x42\x00\x43\x00\x44\x00\x45\x00\x46\x00\x47\x00\x48\x00\x49\x00\x4A\x00\x4B\x00\x4C\x00\x4D\x00\x4E\x00\x4F\x00"
"\x50\x00\x51\x00\x52\x00\x53\x00\x54\x00\x55\x00\x56\x00\x57\x00\x58\x00\x59\x00\x5A\x00\x5B\x00\x5C\x00\x5D\x00\x5E\x00\x5F\x00"
"\x60\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x66\x00\x67\x00\x68\x00\x69\x00\x6A\x00\x6B\x00\x6C\x00\x6D\x00\x6E\x00\x6F\x00"
"\x70\x00\x71\x00\x72\x00\x73\x00\x74\x00\x75\x00\x76\x00\x77\x00\x78\x00\x79\x00\x7A\x00\x7B\x00\x7C\x00\x7D\x00\x7E\x00\x7F\x00"
"\x80\x00\x81\x00\x82\x00\x83\x00\x84\x00\x85\x00\x86\x00\x87\x00\x88\x00\x89\x00\x8A\x00\x8B\x00\x8C\x00\x8D\x00\x8E\x00\x8F\x00"
"\x90\x00\x91\x00\x92\x00\x93\x00\x94\x00\x95\x00\x96\x00\x97\x00\x98\x00\x99\x00\x9A\x00\x9B\x00\x9C\x00\x9D\x00\x9E\x00\x9F\x00"
"\xA0\x00\xA1\x00\xA2\x00\xA3\x00\xA4\x00\xA5\x00\xA6\x00\xA7\x00\xA8\x00\xA9\x00\xAA\x00\xAB\x00\xAC\x00\xAD\x00\xAE\x00\xAF\x00"
"\xB0\x00\xB1\x00\xB2\x00\xB3\x00\xB4\x00\xB5\x00\xB6\x00\xB7\x00\xB8\x00\xB9\x00\xBA\x00\xBB\x00\xBC\x00\xBD\x00\xBE\x00\xBF\x00"
"\xC0\x00\xC1\x00\xC2\x00\xC3\x00\xC4\x00\xC5\x00\xC6\x00\xC7\x00\xC8\x00\xC9\x00\xCA\x00\xCB\x00\xCC\x00\xCD\x00\xCE\x00\xCF\x00"
"\xD0\x00\xD1\x00\xD2\x00\xD3\x00\xD4\x00\xD5\x00\xD6\x00\xD7\x00\xD8\x00\xD9\x00\xDA\x00\xDB\x00\xDC\x00\xDD\x00\xDE\x00\xDF\x00"
"\xE0\x00\xE1\x00\xE2\x00\xE3\x00\xE4\x00\xE5\x00\xE6\x00\xE7\x00\xE8\x00\xE9\x00\xEA\x00\xEB\x00\xEC\x00\xED\x00\xEE\x00\xEF\x00"
"\xF0\x00\xF1\x00\xF2\x00\xF3\x00\xF4\x00\xF5\x00\xF6\x00\xF7\x00\xF8\x00\xF9\x00\xFA\x00\xFB\x00\xFC\x00\xFD\x00\xFE\x00\xFF\x00";

static int _index = 0;

STRING_MAKE STRING_make_buffer;
#define _make STRING_make_buffer

static iconv_t _conv_unicode_utf8 = (iconv_t)-1;
static iconv_t _conv_utf8_unicode = (iconv_t)-1;


/****************************************************************************

	String pool management

****************************************************************************/

#define SIZE_INC 16
#define REAL_SIZE(_len) (((_len) + (SIZE_INC - 1)) & ~(SIZE_INC - 1))

#define POOL_SIZE  16
#define POOL_MAX   64

#define POOL_MAX_LEN   (POOL_SIZE * SIZE_INC)

static STRING *_pool[POOL_SIZE] = { 0 };
static char _pool_count[POOL_SIZE] = { 0 };

#ifdef DEBUG_ME

static STRING *alloc_string(_len) \
{ \
	STRING *str; \
	int size = REAL_SIZE((_len) + 1 + sizeof(STRING)); \
	int pool = (size / SIZE_INC) - 1; \
	\
	MEMORY_count++; \
	\
	if (pool < POOL_SIZE && (_pool_count[pool])) \
	{ \
		str = _pool[pool]; \
		fprintf(stderr, "alloc_string: (%p) %d bytes from pool %d [%d]\n", str, size, pool, _pool_count[pool]); \
		_pool[pool] = *((STRING **)str); \
		_pool_count[pool]--; \
	} \
	else \
	{ \
		str = _my_malloc(size); \
		if (!str) \
			THROW_MEMORY(); \
	} \
	str->len = (_len); \
	str->ref = 1; \
	return str; \
}

#else

#define alloc_string(_len) \
({ \
	STRING *str; \
	int pool = (((_len) + 1 + sizeof(STRING) + (SIZE_INC - 1)) / SIZE_INC) - 1; \
	\
	MEMORY_count++; \
	\
	if (pool < POOL_SIZE && (_pool_count[pool])) \
	{ \
		str = _pool[pool]; \
		_pool[pool] = *((STRING **)str); \
		_pool_count[pool]--; \
	} \
	else \
	{ \
		str = _my_malloc(REAL_SIZE((_len) + 1 + sizeof(STRING))); \
		if (!str) \
			THROW_MEMORY(); \
	} \
	str->len = (_len); \
	str->ref = 1; \
	str; \
})

#endif

extern char *STRING_utf8_current;

void STRING_free_real(char *ptr)
{
	STRING *str = STRING_from_ptr(ptr);
	int size = REAL_SIZE(str->len + 1 + sizeof(STRING));
	int pool = (size / SIZE_INC) - 1;

	if (STRING_utf8_current == ptr)
	{
		//fprintf(stderr, "free STRING_utf8_current (%p)\n", ptr);
		STRING_utf8_current = NULL;
	}
	
	MEMORY_count--;
	
	if (pool < POOL_SIZE)
	{
		if (_pool_count[pool] < POOL_MAX)		
		{
			#ifdef DEBUG_ME
			fprintf(stderr, "STRING_free_real: (%p / %p) %d bytes to pool %d\n", str, ptr, size, pool);
			str->ref = 0x87654321;
			str->len = 0x87654321;
			#endif
			*((STRING **)str) = _pool[pool];
			_pool[pool] = str;
			_pool_count[pool]++;
			return;
		}
	}
	
	_my_free(str);
}

static STRING *realloc_string(STRING *str, int new_len)
{
	int size;
	int new_size;
	
	if (new_len == str->len)
		return str;
		
	size = REAL_SIZE(str->len + 1 + sizeof(STRING));
	new_size = REAL_SIZE(new_len + 1 + sizeof(STRING));
	
	if (new_size != size)
	{
		if (new_len == 0)
		{
			STRING_free_real(str->data);
			return NULL;
		}
		else if (size > POOL_MAX_LEN && new_size > POOL_MAX_LEN)
		{
			str = _my_realloc(str, new_size);
			//REALLOC(&str, new_size, "realloc_string");
		}
		else
		{
			STRING *nstr = alloc_string(new_len);
			if (new_len < str->len)
				memcpy(nstr->data, str->data, new_len);
			else
				memcpy(nstr->data, str->data, str->len);
			if (str->ref == 1)
				STRING_free_real(str->data);
			else
				str->ref--;
			str = nstr;
		}
	}
	
	str->len = new_len;
	return str;
}

static void clear_pool(void)
{
	int i;
	STRING *str, *next;
	
	for (i = 0; i < POOL_SIZE; i++)
	{
		#ifdef DEBUG_ME
		fprintf(stderr, "clear_pool: clear pool #%d\n", i);
		#endif
		str = _pool[i];
		while (str)
		{
			next = *((STRING **)str);
			#ifdef DEBUG_ME
			fprintf(stderr, "%p\n", str);
			#endif
			_my_free(str);
			str = next;
		}
	}
}

/****************************************************************************

	String routines

****************************************************************************/

char *STRING_new(const char *src, int len)
{
	STRING *str;
	char *data;

	if (len == 0)
		return NULL;

	//ALLOC(&str, REAL_SIZE(len + 1 + sizeof(STRING)), "STRING_new");
	str = alloc_string(len);
	data = str->data;

	if (src)
		memcpy(data, src, len);

	data[len] = 0;

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_new %p ( 0 ) \"%.*s\"\n", data, len, src);
	fflush(stderr);
	#endif
	
	return data;
}

char *STRING_free_later(char *ptr)
{
	/*if (NLast >= MAX_LAST_STRING)
		THROW(E_STRING);*/

	//static int nfl = 0;
	
	if (ptr)
	{
		//nfl++;
		//fprintf(stderr, "% 8d % 6d\n", nfl, STRING_length(ptr));
		
		#ifdef DEBUG_ME
		if (STRING_last[_index])
		{
			DEBUG_where();
			fprintf(stderr, "STRING_free_later: release temp: %p '%s'\n", STRING_last[_index], STRING_last[_index]);
			fflush(stderr);
		}
		#endif

		//if (STRING_last[_index] && STRING_length(STRING_last[_index]) >= 1024)
		//	fprintf(stderr, "STRING_free_later: free [%d] %d\n", _index, STRING_length(STRING_last[_index]));
		
		STRING_unref(&STRING_last[_index]);

		#ifdef DEBUG_ME
		fprintf(stderr, "STRING_free_later: post temp: %p '%s'\n", ptr, ptr);
		fflush(stderr);
		#endif

		STRING_last[_index] = ptr;
		//if (STRING_length(ptr) >= 1024)
		//	fprintf(stderr, "STRING_free_later: [%d] = %d\n", _index, STRING_length(ptr));

		_index++;

		if (_index >= STRING_last_count)
			_index = 0;
	}
	
	return ptr;
}


int STRING_get_free_index(void)
{
	return _index;
}


void STRING_clear_cache(void)
{
	int i;
	
	for (i = 0; i < STRING_last_count; i++)
	{
		#ifdef DEBUG_ME
		if (STRING_last[i])
			fprintf(stderr, "release temp %p '%s'\n", STRING_last[i], STRING_last[i]);
		#endif
		STRING_unref(&STRING_last[i]);
		STRING_last[i] = NULL;
	}
	
	_index = 0;
	
	clear_pool();
}


void STRING_exit(void)
{
	STRING_clear_cache();
	
	#ifdef DEBUG_ME
	fprintf(stderr, "STRING_exit\n");
	#endif
	
	if (_conv_unicode_utf8 != ((iconv_t)-1))
		iconv_close(_conv_unicode_utf8);
	if (_conv_utf8_unicode != ((iconv_t)-1))
		iconv_close(_conv_utf8_unicode);
}


char *STRING_extend(char *str, int new_len)
{
	STRING *sstr;

	if (!str)
	{
		sstr = alloc_string(new_len);
		#ifdef DEBUG_ME
			fprintf(stderr, "STRING_extend: NULL -> %p / %p\n", sstr, sstr->data);
		#endif
	}
	else
	{
		sstr = realloc_string(STRING_from_ptr(str), new_len);
		#ifdef DEBUG_ME
			fprintf(stderr, "STRING_extend: %p / %p -> %p / %p\n", STRING_from_ptr(str), str, sstr, sstr->data);
		#endif
	}

	return sstr ? sstr->data : NULL;
}


bool STRING_extend_will_realloc(char *str, int new_len)
{
	STRING *sstr;
	int size;
	int new_size;
	
	if (!str)
		return new_len != 0;
	
	sstr = STRING_from_ptr(str);
	
	if (new_len == sstr->len)
		return FALSE;
		
	size = REAL_SIZE(sstr->len + 1 + sizeof(STRING));
	new_size = REAL_SIZE(new_len + 1 + sizeof(STRING));
	return size != new_size;
}


void STRING_new_temp_value(VALUE *value, const char *src, int len)
{
	value->_string.addr = STRING_new_temp(src, len);

	value->_string.len = STRING_length(value->_string.addr);
	value->_string.start = 0;
	value->type = T_STRING;
}


void STRING_new_constant_value(VALUE *value, const char *src, int len)
{
	value->_string.addr = (char *)src;
	value->_string.len = ((len < 0) ? strlen(src) : len);
	value->_string.start = 0;
	value->type = T_CSTRING;
}


void STRING_void_value(VALUE *value)
{
	value->type = T_CSTRING;
	value->_string.addr = NULL;
	value->_string.start = 0;
	value->_string.len = 0;
}


#if DEBUG_STRING

void STRING_free(char **ptr)
{
	if (*ptr == NULL)
		return;

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_free: %p\n", *ptr);
	fflush(stderr);
	#endif

	STRING_free_real(*ptr);
	*ptr = NULL;
}

void STRING_ref_real(char *ptr)
{
	STRING *str;

	if (ptr == NULL)
		return;

	if (ptr == STRING_watch)
		BREAKPOINT();

	str = STRING_from_ptr(ptr);

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_ref: %p ( %d -> %d )\n", ptr, str->ref, str->ref + 1);
	if (str->ref < 0 || str->ref > 10000)
		fprintf(stderr, "*** BAD\n");
	fflush(stderr);
	#endif

	str->ref++;
}


void STRING_unref_real(char **ptr)
{
	STRING *str;

	if (*ptr == NULL)
		return;

	if (*ptr == STRING_watch)
		BREAKPOINT();

	str = STRING_from_ptr(*ptr);

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_unref: %p ( %d -> %d )\n", *ptr, str->ref, str->ref - 1);
	if (str->ref < 1 || str->ref > 10000)
	{
		fprintf(stderr, "*** BAD\n");
		BREAKPOINT();
	}
	fflush(stderr);
	#endif

	if ((--str->ref) <= 0)
		STRING_free(ptr);
}

#endif

void STRING_unref_keep(char **ptr)
{
	STRING *str;

	if (*ptr == NULL)
		return;

	str = STRING_from_ptr(*ptr);
	if (str->ref > 1)
		str->ref--;
	else
		STRING_free_later(*ptr);
}

/* The get_param argument index starts at 1, not 0! */

#define INDEX_AT     0
#define INDEX_IGNORE (-1)
#define INDEX_ERROR  (-2)

static int get_param_index(const char *str, int len, uint *pos, int *len_pattern)
{
	uint i;
	int index;
	bool err;
	uchar d;
	
	i = *pos + 1;
	d = str[i];
	if (d == '&')
	{
		index = INDEX_AT;
	}
	else if (d >= '1' && d <= '9')
	{
		index = d - '0';
	}
	else if (d == '{')
	{
		err = FALSE;
		index = 0;
		for(;;)
		{
			i++;
			if (i >= len)
				break;
			d = str[i];
			if (d == '}')
				break;
			if (d >= '0' && d <= '9')
				index = index * 10 + d - '0';
			else
				err = TRUE;
		}
		if (err || index < 1 || index >= 64)
			index = INDEX_ERROR;
	}
	else
	{
		index = INDEX_IGNORE;
	}
	
	if (len_pattern)
		*len_pattern = i - *pos + 1;
		
	*pos = i;
	return index;
}

char *STRING_subst(const char *str, int len, SUBST_FUNC get_param)
{
	uint i;
	uchar c;
	int np, lenp;
	int len_subst;
	char *subst;
	char *ps;
	char *p[64];
	int lp[64];

	if (!str)
		return NULL;

	if (len <= 0)
		len = strlen(str);

	// Calculate the length
	
	len_subst = len;
	
	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '&')
		{
			np = get_param_index(str, len, &i, &lenp);
			len_subst -= lenp;
			
			switch (np)
			{
				case INDEX_AT:
					len_subst++;
					break;
				case INDEX_IGNORE:
					len_subst += lenp;
					break;
				case INDEX_ERROR:
					break;
				default:
					np--;
					(*get_param)(np + 1, &p[np], &lp[np]);
					if (lp[np] < 0)
						lp[np] = strlen(p[np]);
					len_subst += lp[np];
			}
		}
	}
	
	subst = STRING_new(NULL, len_subst);
	ps = subst;

	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '&')
		{
			np = get_param_index(str, len, &i, &lenp);
			switch (np)
			{
				case INDEX_AT:
					*ps++ = '&';
					break;
				case INDEX_IGNORE:
					*ps++ = '&';
					*ps++ = str[i];
					break;
				case INDEX_ERROR:
					break;
				default:
					np--;
					memcpy(ps, p[np], lp[np]);
					ps += lp[np];
			}
		}
		else
			*ps++ = c;
	}
	
	*ps = 0;
	return STRING_free_later(subst);
}

char *STRING_subst_add(const char *str, int len, SUBST_ADD_FUNC add_param)
{
	uint i;
	char c;
	int np;

	if (!str)
		return NULL;

	if (len <= 0)
		len = strlen(str);

	STRING_start_len(len);
	
	for (i = 0; i < len; i++)
	{
		c = str[i];
		if (c == '&')
		{
			np = get_param_index(str, len, &i, NULL);
			switch (np)
			{
				case INDEX_AT:
					STRING_make_char('&');
					break;
				case INDEX_IGNORE:
					STRING_make_char('&');
					STRING_make_char(str[i]);
					break;
				case INDEX_ERROR:
					break;
				default:
					(*add_param)(np);
			}
		}
		else
			STRING_make_char(c);
	}
	
	return STRING_end_temp();
}


char *STRING_add(char *str, const char *src, int len)
{
	int old_len;

	if (len <= 0 && src != NULL)
		len = strlen(src);

	if (len <= 0)
		return str;

	old_len = STRING_length(str);

	str = STRING_extend(str, old_len + len);
	if (src)
	{
		memcpy(&str[old_len], src, len);
		str[old_len + len] = 0;
	}
	
	return str;
}


char *STRING_add_char(char *str, char c)
{
	int len = STRING_length(str);
	char *p;

	str = STRING_extend(str, len + 1);
		
	p = str + len;
	p[0] = c;
	p[1] = 0;
	
	return str;
}


int STRING_search(const char *ps, int ls, const char *pp, int lp, int is, bool right, bool nocase)
{
	int pos, ip;

	if (lp > ls)
		return 0;

	if (is < 0)
		is += ls;
	else if (is == 0)
		is = right ? ls : 1;
	
	ls = ls - lp + 1; /* Longueur du début du texte où effectuer la recherche */

	if (is > ls)
	{
		if (!right)
			return 0;
		else
			is = ls;
	}
	else if (is < lp)
	{
		if (right)
			return 0;
		else if (is < 1)
			is = 1;
	}

	is--;

	if (lp == 1)
	{
		char cp = *pp;
		
		if (nocase)
		{
			cp = tolower(cp);
			
			if (right)
			{
				for (; is >= 0; is--)
				{
					if (tolower(ps[is]) == cp)
						return is + 1;
				}
			}
			else
			{
				for (; is < ls; is++)
				{
					if (tolower(ps[is]) == cp)
						return is + 1;
				}
			}
		}
		else
		{
			if (right)
			{
				for (; is >= 0; is--)
				{
					if (ps[is] == cp)
						return is + 1;
				}
			}
			else
			{
				for (; is < ls; is++)
				{
					if (ps[is] == cp)
						return is + 1;
				}
			}
		}
		
		return 0;
	}
	
	pos = 0;
	ps += is;
	
	if (right)
	{
		if (nocase)
		{
			for (; is >= 0; is--, ps--)
			{
				if (tolower(ps[0]) != tolower(pp[0]))
					goto __NEXT_RN;
				
				for (ip = 1; ip < lp; ip++)
				{
					if (tolower(ps[ip]) != tolower(pp[ip]))
						goto __NEXT_RN;
				}
	
				pos = is + 1;
				goto __FOUND;
	
		__NEXT_RN:
			;
			}
		}
		else
		{
			for (; is >= 0; is--, ps--)
			{
				if (ps[0] != pp[0])
					goto __NEXT_R;
				
				for (ip = 1; ip < lp; ip++)
				{
					if (ps[ip] != pp[ip])
						goto __NEXT_R;
				}
	
				pos = is + 1;
				goto __FOUND;
	
		__NEXT_R:
			;
			}
		}
	}
	else
	{
		if (nocase)
		{
			for (; is < ls; is++, ps++)
			{
				if (tolower(ps[0]) != tolower(pp[0]))
					goto __NEXT_LN;
				
				for (ip = 1; ip < lp; ip++)
				{
					if (tolower(ps[ip]) != tolower(pp[ip]))
						goto __NEXT_LN;
				}
	
				pos = is + 1;
				goto __FOUND;
	
		__NEXT_LN:
			;
			}
		}
		else
		{
			for (; is < ls; is++, ps++)
			{
				if (ps[0] != pp[0])
					goto __NEXT_L;
				
				for (ip = 1; ip < lp; ip++)
				{
					if (ps[ip] != pp[ip])
						goto __NEXT_L;
				}
	
				pos = is + 1;
				goto __FOUND;
	
		__NEXT_L:
			;
			}
		}
	}

__FOUND:

	return pos;
}

void STRING_start_len(int len)
{
	_make.inc = 32;
	
	if (len == 0)
		len = 32;
		
	_make.buffer = STRING_new(NULL, len);
	_make.max = len;
	_make.len = 0;
	_make.ptr = _make.buffer;
	_make.ntemp = 0;
}

void STRING_make_dump()
{
	int n = _make.ntemp;
	_make.ntemp = 0;
	STRING_make(_make.temp, n);
}

// len == 0 est possible ! On peut vouloir ajouter une chaîne vide.

void STRING_make(const char *src, int len)
{
  int pos;

	if (!src)
		return;

	if (len < 0)
		len = strlen(src);

  if (len <= 0)
    return;
    
	if (_make.ntemp)
		STRING_make_dump();

	_make.len += len;

	if (_make.len >= _make.max)
	{
		pos = (_make.len - _make.max) * 4;
		if (pos < _make.inc)
			pos = _make.inc;
		else
		{
			_make.inc = (pos + 31) & ~31;
			if (_make.inc > 1024)
				_make.inc = 1024;
		}
		
		_make.max += pos;
		
		//fprintf(stderr, "STRING_extend: %d\n", _max - STRING_length(SUBST_buffer));
		pos = _make.ptr - _make.buffer;
		_make.buffer = STRING_extend(_make.buffer, _make.max);
		_make.ptr = _make.buffer + pos;
	}

	memcpy(_make.ptr, src, len);
	_make.ptr += len;
}

char *STRING_end()
{
	if (_make.ntemp)
		STRING_make_dump();
	
	if (_make.len)
	{
		_make.buffer = STRING_extend(_make.buffer, _make.len);
		_make.buffer[_make.len] = 0;
	}
	else
		STRING_free(&_make.buffer);
	
	return _make.buffer;
}

char *STRING_end_temp()
{
	STRING_end();
	
	if (_make.buffer)
		STRING_free_later(_make.buffer);
	
	return _make.buffer;
}

/****************************************************************************

	Charset conversion routines

****************************************************************************/

static iconv_t my_iconv_open(const char *dst, const char *src)
{
	const char *osrc, *odst;
	iconv_t *cache;
	iconv_t handle;
	
	osrc = src;
	odst = dst;
	
	if (dst == SC_UNICODE)
		dst = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";
	else if (dst == SC_UTF8)
		dst = "UTF-8";
	else if (!dst || *dst == 0)
		dst = "ASCII";
		
	if (src == SC_UNICODE)
		src = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";
	else if (src == SC_UTF8)
		src = "UTF-8";
	else if (!src || *src == 0)
		src = "ASCII";

	if (osrc == SC_UNICODE && odst == SC_UTF8)
		cache = &_conv_unicode_utf8;
	else if (osrc == SC_UTF8 && odst == SC_UNICODE)
		cache = &_conv_utf8_unicode;
	else
		cache = NULL;
	
	if (cache && *cache != ((iconv_t)-1))
		return *cache;
	
	//fprintf(stderr, "iconv_open: %s -> %s\n", src, dst);
	handle = iconv_open(dst, src);
	if (cache)
		*cache = handle;
	return handle;
}


static void my_iconv_close(iconv_t handle)
{
	if (handle == _conv_unicode_utf8 || handle == _conv_utf8_unicode)
		return;
	
	iconv_close(handle);
}


int STRING_conv(char **result, const char *str, int len, const char *src, const char *dst, bool throw)
{
	iconv_t handle;
	bool err;
	const char *in;
	char *out;
	size_t in_len;
	size_t out_len;
	size_t ret;
	int errcode = 0;
	bool unicode;

	*result = NULL;

	in = str;
	in_len = len;

	if (len == 0)
		return errcode;

	unicode = (dst == SC_UNICODE);
	
	handle = my_iconv_open(dst, src);
	if (handle == (iconv_t)(-1))
	{
		if (errno == EINVAL)
			errcode = E_UCONV;
		else
			errcode = E_CONV;
	}
	else
	{
		err = FALSE;
	
		for(;;)
		{
			out = COMMON_buffer;
			out_len = COMMON_BUF_MAX;
	
			#if defined(OS_SOLARIS) || defined(OS_FREEBSD) || defined(OS_OPENBSD)
			ret = iconv(handle, &in, &in_len, &out, &out_len);
			#else
			ret = iconv(handle, (char **)&in, &in_len, &out, &out_len);
			#endif
	
			if (ret != (size_t)(-1) || errno == E2BIG)
				*result = STRING_add(*result, COMMON_buffer, COMMON_BUF_MAX - out_len);
	
			if (ret != (size_t)(-1))
				break;
	
			if (errno != E2BIG)
			{
				err = TRUE;
				break;
			}
		}
		
		my_iconv_close(handle);
	
		if (unicode)
			*result = STRING_add(*result, "\0\0\0", 3);
	
		STRING_extend_end(*result);
	
		if (err)
			errcode = E_CONV;
	}
	
	if (throw && errcode)
		THROW(errcode);

	return errcode;
}


char *STRING_conv_to_UTF8(const char *name, int len)
{
	char *result = NULL;

	if (!name)
		return "";

	if (LOCAL_is_UTF8)
	{
		if (len <= 0)
			result = (char *)name;
		else
			result = STRING_new_temp(name, len);
	}
	else
	{
		if (len <= 0)
			len = strlen(name);

		STRING_conv(&result, name, len, LOCAL_encoding, SC_UTF8, TRUE);
	}

	if (result)
		return result;
	else
		return "";
}


char *STRING_conv_file_name(const char *name, int len)
{
	char *result = NULL;
	int pos;
	struct passwd *info;
	char *dir;
	char *user;
	int err;

	if (!name)
		return "";

	if (len <= 0)
		len = strlen(name);

	if (len > 0 && *name == '~')
	{
		for (pos = 0; pos < len; pos++)
		{
			if (name[pos] == '/')
				break;
		}

		if (pos <= 1)
			dir = PROJECT_get_home();
		else
		{
			user = STRING_new_temp(&name[1], pos - 1);
			info = getpwnam(user);
			if (info)
				dir = info->pw_dir;
			else
				dir = NULL;
		}

		if (dir)
		{
			user = STRING_new_zero(dir);
			if (pos < len)
				user = STRING_add(user, &name[pos], len - pos);
			name = user;
			len = STRING_length(name);
			STRING_free_later(user);
		}
	}

	if (LOCAL_is_UTF8)
		result = STRING_new_temp(name, len);
	else
	{
		err = STRING_conv(&result, name, len, SC_UTF8, LOCAL_encoding, FALSE);
		if (err)
			result = STRING_new_temp(name, len);
	}
	//fprintf(stderr, "STRING_conv_file_name: %s\n", result);

	if (result)
		return result;
	else
		return "";
}


/****************************************************************************

	Common string routines

****************************************************************************/

#include "gb_common_string_temp.h"


