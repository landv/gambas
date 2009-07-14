/***************************************************************************

	String.c

	The String management routines

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

#if DEBUG_STRING
#define DEBUG_ME
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

#define STRING_last_count 32
static char *STRING_last[STRING_last_count] = { 0 };

static const char _char_string[512] = 
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

//static HASH_TABLE *_intern = NULL;

/****************************************************************************

	String pool management

****************************************************************************/

#define SIZE_INC 16
#define REAL_SIZE(_len) (((_len) + (SIZE_INC - 1)) & ~(SIZE_INC - 1))

#define POOL_SIZE  16
#define POOL_MAX   32

#define POOL_MAX_LEN   (POOL_SIZE * SIZE_INC)

static STRING *_pool[POOL_SIZE] = { 0 };
static int _pool_count[POOL_SIZE] = { 0 };

static STRING *alloc_string(int len)
{
	STRING *str;
	int size = REAL_SIZE(len + 1 + sizeof(STRING));
	int pool = (size / SIZE_INC) - 1;
	
	if (pool < POOL_SIZE)
	{
		if (_pool_count[pool])
		{
			str = _pool[pool];
			#ifdef DEBUG_ME
			fprintf(stderr, "alloc_string: %d bytes from pool %d -> %p\n", size, pool, str);
			#endif
			_pool[pool] = *((STRING **)str);
			_pool_count[pool]--;
			str->len = len;
			str->ref = 1;
			return str;
		}
		//else
		//	printf("pool[%d] void\n", pool);
	}
	//else
	//	printf("alloc_string: %d\n", pool);
			
	ALLOC(&str, size, "alloc_string");
	str->len = len;
	str->ref = 1;
	return str;
}

void STRING_free_real(char *ptr)
{
	STRING *str = STRING_from_ptr(ptr);
	int size = REAL_SIZE(str->len + 1 + sizeof(STRING));
	int pool = (size / SIZE_INC) - 1;

	if (pool < POOL_SIZE)
	{
		if (_pool_count[pool] < POOL_MAX)		
		{
			#ifdef DEBUG_ME
			fprintf(stderr, "STRING_free_real: (%p) %d bytes to pool %d\n", str, size, pool);
			#endif
			*((STRING **)str) = _pool[pool];
			_pool[pool] = str;
			_pool_count[pool]++;
			return;
		}
	}
	
	IFREE(str, "free_string");
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
			REALLOC(&str, new_size, "realloc_string");
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
		str = _pool[i];
		while (str)
		{
			next = *((STRING **)str);
			IFREE(str, "clear_pool");
			str = next;
		}
	}
}

/****************************************************************************

	String routines

****************************************************************************/

void STRING_new(char **ptr, const char *src, int len)
{
	STRING *str;

	if (len <= 0 && src != NULL)
		len = strlen(src);

	if (len <= 0)
	{
		*ptr = NULL;
		return;
	}

	//ALLOC(&str, REAL_SIZE(len + 1 + sizeof(STRING)), "STRING_new");
	str = alloc_string(len);

	if (src)
		memcpy(str->data, src, len);

	str->data[len] = 0;

	*ptr = str->data;

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_new %p ( 0 ) \"%.*s\"\n", *ptr, len, src);
	fflush(stderr);
	#endif
}

void STRING_free_later(char *ptr)
{
	/*if (NLast >= MAX_LAST_STRING)
		THROW(E_STRING);*/

	#ifdef DEBUG_ME
	if (STRING_last[_index])
	{
		DEBUG_where();
		fprintf(stderr, "STRING_free_later: release temp: %p '%s'\n", STRING_last[_index], STRING_last[_index]);
		fflush(stderr);
	}
	#endif

	STRING_unref(&STRING_last[_index]);

	#ifdef DEBUG_ME
	fprintf(stderr, "STRING_free_later: post temp: %p '%s'\n", ptr, ptr);
	fflush(stderr);
	#endif

	STRING_last[_index] = ptr;

	_index++;

	if (_index >= STRING_last_count)
		_index = 0;
}


int STRING_get_free_index(void)
{
	return _index;
}



/*void STRING_init(void)
{
	HASH_TABLE_create(&_intern, 0, GB_COMP_TEXT);
}*/

void STRING_exit(void)
{
	int i;

	for (i = 0; i < STRING_last_count; i++)
	{
		/*if (STRING_last[i])
			printf("release temp %p '%s'\n", STRING_last[i], STRING_last[i]);*/
		STRING_unref(&STRING_last[i]);
		STRING_last[i] = NULL;
	}
	
	_index = 0;
	
	clear_pool();
}


void STRING_extend(char **ptr, int new_len)
{
	STRING *str;

	if (!*ptr)
		str = alloc_string(new_len);
	else
		str = realloc_string(STRING_from_ptr(*ptr), new_len);

	*ptr = str ? str->data : NULL;
}


void STRING_extend_end(char **ptr)
{
	if (*ptr)
	{
		(*ptr)[STRING_length(*ptr)] = 0;
		STRING_free_later(*ptr);
	}
}


void STRING_copy_from_value_temp(char **ptr, VALUE *value)
{
	if (value->_string.len == 0)
		*ptr = NULL;
	else if (value->type == T_STRING && value->_string.start == 0 && value->_string.len == STRING_length(value->_string.addr))
		*ptr = value->_string.addr;
	else
		STRING_new_temp(ptr, &value->_string.addr[value->_string.start], value->_string.len);
}


/* Attention ! Contrairement �STRING_new, STRING_new_temp_value cr� des
	cha�es temporaires.
*/

void STRING_new_temp_value(VALUE *value, const char *src, int len)
{
	STRING_new_temp(&(value->_string.addr), src, len);

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


void STRING_char_value(VALUE *value, uchar car)
{
	value->type = T_CSTRING;
	value->_string.addr = (char *)&_char_string[(int)car * 2];
	value->_string.start = 0;
	value->_string.len = 1;
}

#if DEBUG_STRING

void STRING_free(char **ptr)
{
	if (*ptr == NULL)
		return;

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_free: %p %p\n", *ptr, ptr);
	fflush(stderr);
	#endif

	STRING_free_real(*ptr);
	*ptr = NULL;
}

void STRING_ref(char *ptr)
{
	STRING *str;

	if (ptr == NULL)
		return;

	str = STRING_from_ptr(ptr);

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_ref: %p ( %ld -> %ld )\n", ptr, str->ref, str->ref + 1);
	if (str->ref < 0 || str->ref > 10000)
		fprintf(stderr, "*** BAD\n");
	fflush(stderr);
	#endif

	str->ref++;
}


void STRING_unref(char **ptr)
{
	STRING *str;

	if (*ptr == NULL)
		return;

	str = STRING_from_ptr(*ptr);

	#ifdef DEBUG_ME
	DEBUG_where();
	fprintf(stderr, "STRING_unref: %p ( %ld -> %ld )\n", *ptr, str->ref, str->ref - 1);
	if (str->ref < 1 || str->ref > 10000)
		fprintf(stderr, "*** BAD\n");
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
	
	STRING_new(&subst, NULL, len_subst);
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
	STRING_free_later(subst);
	return subst;
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


void STRING_add(char **ptr, const char *src, int len)
{
	int old_len;

	if (len <= 0 && src != NULL)
		len = strlen(src);

	if (len <= 0)
		return;

	old_len = STRING_length(*ptr);

	STRING_extend(ptr, old_len + len);
	if (src)
	{
		memcpy(&((*ptr)[old_len]), src, len);
		(*ptr)[old_len + len] = 0;
	}
}


void STRING_add_char(char **ptr, char c)
{
	int len = STRING_length(*ptr);
	char *p;

	//if (!(len & (SIZE_INC - 1)))
	STRING_extend(ptr, len + 1);
	//else
	//STRING_from_ptr(*ptr)->len++;
		
	p = *ptr + len;
	p[0] = c;
	p[1] = 0;
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

	if (dst == SC_UNICODE)
	{
		dst = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";
		unicode = TRUE;
	}
	else
	{
		unicode = FALSE;

		if (!dst || *dst == 0)
			dst = "ASCII";
	}

	if (src == SC_UNICODE)
	{
		src = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";
	}
	else
	{
		if (!src || *src == 0)
			src = "ASCII";
	}

	handle = iconv_open(dst, src);
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
				STRING_add(result, COMMON_buffer, COMMON_BUF_MAX - out_len);
	
			if (ret != (size_t)(-1))
				break;
	
			if (errno != E2BIG)
			{
				err = TRUE;
				break;
			}
		}
	
		iconv_close(handle);
	
		if (unicode)
			STRING_add(result, "\0\0\0", sizeof(wchar_t) - 1);
	
		STRING_extend_end(result);
	
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
			STRING_new_temp(&result, name, len);
	}
	else
	{
		if (len <= 0)
			len = strlen(name);

		STRING_conv(&result, name, len, LOCAL_encoding, "UTF-8", TRUE);
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
			STRING_new_temp(&user, &name[1], pos - 1);
			info = getpwnam(user);
			if (info)
				dir = info->pw_dir;
			else
				dir = NULL;
		}

		if (dir)
		{
			STRING_new(&user, dir, 0);
			if (pos < len)
				STRING_add(&user, &name[pos], len - pos);
			name = user;
			len = STRING_length(name);
			STRING_free_later(user);
		}
	}

	if (LOCAL_is_UTF8)
		STRING_new_temp(&result, name, len);
	else
		STRING_conv(&result, name, len, "UTF-8", LOCAL_encoding, TRUE);

	//fprintf(stderr, "STRING_conv_file_name: %s\n", result);

	if (result)
		return result;
	else
		return "";
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
		uchar cp = *pp;
		
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
				for (ip = 0; ip < lp; ip++)
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
				for (ip = 0; ip < lp; ip++)
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
				for (ip = 0; ip < lp; ip++)
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
				for (ip = 0; ip < lp; ip++)
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
		
	STRING_new(&_make.buffer, NULL, len);
	
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
		STRING_extend(&_make.buffer, _make.max);
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
		STRING_extend(&_make.buffer, _make.len);
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

#include "gb_common_string_temp.h"


