/***************************************************************************

  String.h

  The String management routines

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __STRING_H
#define __STRING_H

#include "gbx_value.h"
#include "gbx_subst.h"
#include "gbx_debug.h"

typedef
  struct
  {
    long len;
    long ref;
    char data[0];
  }
  PACKED
  STRING;

PUBLIC void STRING_init(void);
PUBLIC void STRING_exit(void);

PUBLIC void STRING_new(char **ptr, const char *src, int len);
PUBLIC void STRING_new_temp(char **ptr, const char *src, int len);
PUBLIC void STRING_free(char **ptr);
PUBLIC long STRING_get_free_index(void);

PUBLIC void STRING_extend(char **ptr, int new_len);
PUBLIC void STRING_extend_end(char **ptr);
PUBLIC void STRING_add(char **ptr, const char *src, int len);

PUBLIC void STRING_copy_from_value_temp(char **ptr, VALUE *value);

PUBLIC void STRING_new_temp_value(VALUE *value, const char *src, int len);
PUBLIC void STRING_new_constant_value(VALUE *value, const char *src, int len);

PUBLIC void STRING_char_value(VALUE *value, uchar car);
PUBLIC void STRING_void_value(VALUE *value);

PUBLIC int STRING_comp_value(VALUE *str1, VALUE *str2);
PUBLIC int STRING_comp_value_ignore_case(VALUE *str1, VALUE *str2);

PUBLIC char *STRING_subst(const char *string, long len, SUBST_FUNC get_param);
PUBLIC int STRING_conv(char **result, const char *str, long len, const char *src, const char *dst, bool throw);
PUBLIC char *STRING_conv_file_name(const char *name, long len);
PUBLIC char *STRING_conv_to_UTF8(const char *name, long len);


#define STRING_from_ptr(_ptr) ((STRING *)((_ptr) - offsetof(STRING, data)))
#define STRING_length(_ptr) ((_ptr) == NULL ? 0 : STRING_from_ptr(_ptr)->len)

#if DEBUG_STRING

PUBLIC void STRING_ref(char *ptr);
PUBLIC void STRING_unref(char **ptr);

#else

#define STRING_ref(_p) \
({ \
  char *ptr = _p; \
  if (ptr) \
    STRING_from_ptr(ptr)->ref++; \
})

#define STRING_unref(_p) \
({ \
  char **pptr = _p; \
  char *ptr = *pptr; \
  STRING *str; \
  if (ptr) \
  { \
	  str = STRING_from_ptr(ptr); \
  	if ((--str->ref) <= 0) \
    	STRING_free(pptr); \
  } \
})

#endif

PUBLIC void STRING_unref_keep(char **ptr);

PUBLIC long STRING_search(const char *ps, long ls, const char *pp, long lp, long is, bool right);

#endif /* */
