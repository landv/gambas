/***************************************************************************

  gbx_string.h

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

#ifndef __STRING_H
#define __STRING_H

#include "gbx_value.h"
#include "gbx_debug.h"
#include "gb_common_string.h"

typedef
  struct
  {
    int ref;
    int len;
    char data[0];
  }
  STRING;

typedef
  void (*SUBST_FUNC)(int, char **, int *);

typedef
  void (*SUBST_ADD_FUNC)(int);

#define STRING_MAKE_TEMP 32

typedef
	struct
	{
		char *buffer;
		char *ptr;
		int inc;
		int len;
		int max;
		char temp[STRING_MAKE_TEMP];
		int ntemp;
	}
	STRING_MAKE;

// NOTE: Defined in gambas.h too
#define SC_UNICODE ((char *)-1)
#define SC_UTF8 ((char *)-2)

#ifndef __STRING_C
extern STRING_MAKE STRING_make_buffer;
extern const char STRING_char_string[];
#endif

void STRING_init(void);
void STRING_exit(void);
void STRING_clear_cache(void);

char *STRING_new(const char *src, int len);
#define STRING_new_zero(_src) \
({ \
  const char *_s = (_src); \
	STRING_new(_s, _s ? strlen(_s) : 0); \
})

void STRING_free_real(char *ptr);
char *STRING_free_later(char *ptr);
int STRING_get_free_index(void);

#define STRING_new_temp(_src, _len) STRING_free_later(STRING_new(_src, _len))
#define STRING_new_temp_zero(_src) STRING_free_later(STRING_new_zero(_src))

char *STRING_extend(char *str, int new_len);
bool STRING_extend_will_realloc(char *str, int new_len);

//void STRING_extend_end(char *str);
char *STRING_add(char *str, const char *src, int len);
char *STRING_add_char(char *str, char c);

#define STRING_extend_end(_str) \
do { \
	if (_str) \
	{ \
		(_str)[STRING_length((_str))] = 0; \
		STRING_free_later(_str); \
	} \
} while(0)


#define STRING_copy_from_value(_value) \
({ \
	char *ptr; \
	if ((_value)->_string.len == 0) \
		ptr = NULL; \
	else if ((_value)->type == T_STRING && (_value)->_string.start == 0 && (_value)->_string.len == STRING_length((_value)->_string.addr)) \
		ptr = (_value)->_string.addr; \
	else \
		ptr = STRING_new(&(_value)->_string.addr[(_value)->_string.start], (_value)->_string.len); \
	ptr; \
})

#define STRING_copy_from_value_temp(_value) \
({ \
	char *ptr; \
	if ((_value)->_string.len == 0) \
		ptr = NULL; \
	else if ((_value)->type == T_STRING && (_value)->_string.start == 0 && (_value)->_string.len == STRING_length((_value)->_string.addr)) \
		ptr = (_value)->_string.addr; \
	else \
		ptr = STRING_new_temp(&(_value)->_string.addr[(_value)->_string.start], (_value)->_string.len); \
	ptr; \
})


void STRING_new_temp_value(VALUE *value, const char *src, int len);
void STRING_new_constant_value(VALUE *value, const char *src, int len);

#define STRING_char_value(_value, _car) \
do { \
	_value->type = T_CSTRING; \
	_value->_string.addr = (char *)&STRING_char_string[(_car) * 2]; \
	_value->_string.start = 0; \
	_value->_string.len = 1; \
} while(0)

void STRING_void_value(VALUE *value);

char *STRING_subst(const char *str, int len, SUBST_FUNC get_param);
char *STRING_subst_add(const char *str, int len, SUBST_ADD_FUNC add_param);
int STRING_conv(char **result, const char *str, int len, const char *src, const char *dst, bool throw);
char *STRING_conv_file_name(const char *name, int len);
char *STRING_conv_to_UTF8(const char *name, int len);

#define STRING_from_ptr(_ptr) ((STRING *)((_ptr) - offsetof(STRING, data)))
#define STRING_length(_ptr) ((_ptr) == NULL ? 0 : STRING_from_ptr(_ptr)->len)

#if DEBUG_STRING

#ifndef __STRING_C
extern char *STRING_watch;
#endif

void STRING_free(char **ptr);
void STRING_ref_real(char *ptr);
void STRING_unref_real(char **ptr);

#define STRING_ref(_p) fprintf(stderr, "<< %s >> ", __func__), STRING_ref_real(_p)
#define STRING_unref(_p) fprintf(stderr, "<< %s >> ", __func__), STRING_unref_real(_p)

#else

#define STRING_free(_p) \
({ \
  char **pptr = _p; \
  char *ptr = *pptr; \
  if (LIKELY(ptr != NULL)) \
  { \
  	STRING_free_real(ptr); \
  	*pptr = NULL; \
  } \
})

#define STRING_ref(_p) \
({ \
  char *ptr = _p; \
  if (LIKELY(ptr != NULL)) \
	{ \
    STRING_from_ptr(ptr)->ref++; \
  } \
})

#define STRING_unref(_p) \
({ \
  char **pptr = _p; \
  char *ptr = *pptr; \
  STRING *str; \
  if (LIKELY(ptr != NULL)) \
  { \
	  str = STRING_from_ptr(ptr); \
  	if ((--str->ref) <= 0) \
  	{ \
	  	STRING_free_real(ptr); \
    	*pptr = NULL; \
    } \
  } \
})

#endif

void STRING_unref_keep(char **ptr);

int STRING_search(const char *ps, int ls, const char *pp, int lp, int is, bool right, bool nocase);

void STRING_start_len(int len);
#define STRING_start() STRING_start_len(0)
char *STRING_end();
char *STRING_end_temp();
void STRING_make(const char *src, int len);
void STRING_make_dump();

#define STRING_make_char(_c) \
({ \
	if (UNLIKELY(STRING_make_buffer.ntemp == STRING_MAKE_TEMP)) \
		STRING_make_dump(); \
	STRING_make_buffer.temp[STRING_make_buffer.ntemp++] = (_c); \
})

#endif
