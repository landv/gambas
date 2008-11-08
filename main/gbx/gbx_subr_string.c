/***************************************************************************

  subr_string.c

  The String management subroutines

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

#include "gb_common.h"
#include "gb_common_buffer.h"
#include "gb_common_case.h"

#include <ctype.h>
#include <regex.h>

#include "gb_pcode.h"
#include "gbx_value.h"
#include "gbx_subr.h"
#include "gbx_regexp.h"
#include "gbx_class.h"
#include "gbx_string.h"
#include "gbx_c_array.h"
#include "gbx_local.h"
#include "gbx_compare.h"

void SUBR_cat(void)
{
  int i;
  int len, len_cat;
  char *str, *ptr;

  SUBR_ENTER();

  len_cat = 0;

  for (i = 0; i < NPARAM; i++)
  {
    VALUE_conv_string(&PARAM[i]);
    /*BORROW(&PARAM[i]);*/
    len_cat += PARAM[i]._string.len;
  }

  STRING_new_temp(&str, NULL, len_cat);
  ptr = str;

  for (i = 0; i < NPARAM; i++)
  {
    len = PARAM[i]._string.len;
    if (len > 0)
    {
      /*printf("add %p ", PARAM[i]._string.addr + PARAM[i]._string.start); fflush(NULL);
      printf("%.*s\n", (int)len, PARAM[i]._string.addr + PARAM[i]._string.start);*/
      memcpy(ptr, PARAM[i]._string.addr + PARAM[i]._string.start, len);
      ptr += len;
    }
  }

  /*printf("\n");*/

  RETURN->type = T_STRING;
  RETURN->_string.addr = str;
  RETURN->_string.start = 0;
  RETURN->_string.len = len_cat;

  SUBR_LEAVE();
}


void SUBR_file(void)
{
  int i;
  int length;
  char *addr;
  int len;
  char *str, *ptr;
  boolean slash;

  SUBR_ENTER();

  length = 0;
  slash = FALSE;

  for (i = 0; i < NPARAM; i++)
  {
    /*VALUE_conv(&PARAM[i], T_STRING);*/
    SUBR_get_string_len(&PARAM[i], &addr, &len);

    if (len > 0)
    {
      if (length > 0)
      {
        if (!slash && (addr[0] != '/'))
          length++;
      }

      slash = addr[len - 1] == '/';

      length += len;
    }

  }

  STRING_new_temp(&str, NULL, length);
  ptr = str;

  for (i = 0; i < NPARAM; i++)
  {
    VALUE_get_string(&PARAM[i], &addr, &len);
    if (len > 0)
    {
      if ((ptr > str) && (ptr[-1] != '/') && (*addr != '/'))
        *ptr++ = '/';
      memcpy(ptr, addr, len);
      ptr += len;
    }
  }

  RETURN->type = T_STRING;
  RETURN->_string.addr = str;
  RETURN->_string.start = 0;
  RETURN->_string.len = length;

  SUBR_LEAVE();
}


#if 0
void SUBR_left(void)
{
  int val;

  SUBR_ENTER();

  if (SUBR_check_string(PARAM))
    goto _FIN;

  if (NPARAM == 1)
    val = 1;
  else
  {
    VALUE_conv(&PARAM[1], T_INTEGER);
    val = PARAM[1]._integer.value;
  }

  if (val < 0)
    val += PARAM->_string.len;

  PARAM->_string.len = MinMax(val, 0, PARAM->_string.len);

_FIN:

  SP -= NPARAM;
  SP++;
}



void SUBR_right(void)
{
  int val;
  int new_len;

  SUBR_ENTER();

  if (SUBR_check_string(PARAM))
    goto _FIN;

  if (NPARAM == 1)
    val = 1;
  else
  {
    VALUE_conv(&PARAM[1], T_INTEGER);
    val = PARAM[1]._integer.value;
  }

  if (val < 0)
    val += PARAM->_string.len;

  new_len = MinMax(val, 0, PARAM->_string.len);

  PARAM->_string.start += PARAM->_string.len - new_len;
  PARAM->_string.len = new_len;

_FIN:

  SP -= NPARAM;
  SP++;
}




void SUBR_mid(void)
{
  int start;
  int len;

  SUBR_ENTER();

  if (SUBR_check_string(PARAM))
    goto FIN;

  VALUE_conv(&PARAM[1], T_INTEGER);
  start = PARAM[1]._integer.value - 1;

  if (start < 0)
    THROW(E_ARG);

  if (start >= PARAM->_string.len)
  {
    RELEASE(PARAM);
    STRING_void_value(PARAM);
    goto FIN;
  }

  if (NPARAM == 2)
    len = PARAM->_string.len;
  else
  {
    VALUE_conv(&PARAM[2], T_INTEGER);
    len = PARAM[2]._integer.value;
  }

  if (len < 0)
    len = Max(0, PARAM->_string.len - start + len);

  len = MinMax(len, 0, PARAM->_string.len - start);

  if (len == 0)
  {
    RELEASE(PARAM);
    PARAM->_string.addr = NULL;
    PARAM->_string.start = 0;
  }
  else
    PARAM->_string.start += start;

  PARAM->_string.len = len;

FIN:

  SP -= NPARAM;
  SP++;
}



void SUBR_len(void)
{
  int len;

  SUBR_GET_PARAM(1);

  if (SUBR_check_string(PARAM))
    len = 0;
  else
    len = PARAM->_string.len;

  RELEASE(PARAM);

  PARAM->type = T_INTEGER;
  PARAM->_integer.value = len;
}
#endif

void SUBR_space(void)
{
  int len;

  SUBR_ENTER_PARAM(1);

  SUBR_check_integer(PARAM);
  len = PARAM->_integer.value;

  if (len < 0)
    THROW(E_ARG);

  if (len == 0)
  {
    STRING_void_value(RETURN);
  }
  else
  {
    STRING_new_temp_value(RETURN, NULL, len);
    memset(RETURN->_string.addr, ' ', len);
  }

  SUBR_LEAVE();
}



void SUBR_string(void)
{
  int i;
  char *d;
  char *s;
  int ld, ls;

  SUBR_ENTER_PARAM(2);

  SUBR_check_integer(PARAM);
  SUBR_get_string_len(&PARAM[1], &s, &ls);

  ld = PARAM->_integer.value * ls;
  if (ld < 0)
    THROW(E_ARG);

  if (ld == 0)
  {
    STRING_void_value(RETURN);
  }
  else
  {
    STRING_new_temp_value(RETURN, NULL, ld);
    d = RETURN->_string.addr;

    for (i = 0; i < PARAM->_integer.value; i++)
    {
      memcpy(d, s, ls);
      d += ls;
    }

    *d = 0;
  }

  SUBR_LEAVE();
}


void SUBR_trim(void)
{
  unsigned char *str;
  bool left, right;
  int code;

  SUBR_GET_PARAM(1);

  if (SUBR_check_string(PARAM))
    return;

  code = EXEC_code & 0x1F;
  left = (code == 0 || code == 1);
  right = (code == 0 || code == 2);

 /* if (!(left || right))
    THROW(E_ILLEGAL);*/

  if (PARAM->_string.len > 0)
  {
    str = (uchar *)&PARAM->_string.addr[PARAM->_string.start];

    if (left)
    {
      while (PARAM->_string.len > 0 && *str <= ' ')
      {
        PARAM->_string.start++;
        PARAM->_string.len--;
        str++;
      }
    }

    if (right)
    {
      while (PARAM->_string.len > 0 && str[PARAM->_string.len - 1] <= ' ')
      {
        PARAM->_string.len--;
      }
    }
  }
}




#define STRING_APPLY(_func) \
  char *str; \
  int len, i; \
  \
  SUBR_ENTER_PARAM(1); \
   \
  if (SUBR_check_string(PARAM)) \
    STRING_void_value(RETURN); \
  else \
  { \
    len = PARAM->_string.len; \
    if (len > 0) \
    { \
      STRING_new_temp(&str, &PARAM->_string.addr[PARAM->_string.start], PARAM->_string.len); \
      \
      for (i = 0; i < len; i++) \
        str[i] = _func(str[i]); \
        \
      RETURN->type = T_STRING; \
      RETURN->_string.addr = str; \
      RETURN->_string.start = 0; \
      RETURN->_string.len = len; \
    } \
  } \
  \
  SUBR_LEAVE();


void SUBR_upper(void)
{
  STRING_APPLY(toupper);
}

void SUBR_lower(void)
{
  STRING_APPLY(tolower);
}


void SUBR_chr(void)
{
  int car;

  SUBR_GET_PARAM(1);

  VALUE_conv(PARAM, T_INTEGER);
  /*SUBR_check_integer(PARAM);*/

  car = PARAM->_integer.value;
  if (car < 0 || car > 255)
    THROW(E_ARG);

  STRING_char_value(PARAM, car);
}



void SUBR_asc(void)
{
  int pos = 0;

  SUBR_ENTER();

  if (!SUBR_check_string(PARAM))
  {
    pos = 1;
    if (NPARAM == 2)
    {
      SUBR_check_integer(&PARAM[1]);
      pos = PARAM[1]._integer.value;
    }

    if (pos < 1 || pos > PARAM->_string.len)
      pos = 0;
    else
      pos = (unsigned char)PARAM->_string.addr[PARAM->_string.start + pos - 1];
  }

  RETURN->type = T_INTEGER;
  RETURN->_integer.value = pos;

  SUBR_LEAVE();
}



void SUBR_instr(void)
{
  boolean right, nocase = FALSE;
  int is, pos;
  char *ps, *pp;
  int ls, lp;

  SUBR_ENTER();

  /* Knuth Morris Pratt one day maybe ? */

  pos = 0;

  if (SUBR_check_string(PARAM))
    goto __FOUND;

  if (SUBR_check_string(&PARAM[1]))
    goto __FOUND;

  lp = PARAM[1]._string.len;
  ls = PARAM->_string.len;

  right = ((EXEC_code >> 8) == CODE_RINSTR);

  if (lp > ls) goto __FOUND;

  is = 0;

  if (NPARAM >= 3)
    is = SUBR_get_integer(&PARAM[2]);
  
  if (NPARAM == 4)
    nocase = SUBR_get_integer(&PARAM[3]) == GB_COMP_TEXT;

  ps = PARAM->_string.addr + PARAM->_string.start;
  pp = PARAM[1]._string.addr + PARAM[1]._string.start;

  pos = STRING_search(ps, ls, pp, lp, is, right, nocase);

__FOUND:

  RETURN->type = T_INTEGER;
  RETURN->_integer.value = pos;

  SUBR_LEAVE();
}


void SUBR_like(void)
{
  char *pattern;
  char *string;
  int len_pattern, len_string;
  boolean ret;

  SUBR_ENTER_PARAM(2);

  SUBR_get_string_len(&PARAM[0], &string, &len_string);
  SUBR_get_string_len(&PARAM[1], &pattern, &len_pattern);

  ret = REGEXP_match(pattern, len_pattern, string, len_string) ? -1 : 0;

  RETURN->type = T_BOOLEAN;
  RETURN->_boolean.value = ret;

  SUBR_LEAVE();
}


static int subst_nparam;
static VALUE *subst_param;

static void get_subst(int np, char **str, int *len)
{
  if (np > 0 && np < subst_nparam)
    VALUE_get_string(&subst_param[np], str, len);
  else
  {
    *str = NULL;
    *len = 0;
  }
}


void SUBR_subst(void)
{
  char *string;
  int len;
  int np;

  SUBR_ENTER();

  SUBR_get_string_len(&PARAM[0], &string, &len);

  for (np = 1; np < NPARAM; np++)
    VALUE_conv_string(&PARAM[np]);

  subst_param = PARAM;
  subst_nparam = NPARAM;

  string = STRING_subst(string, len, get_subst);

  /*for (np = 0; np < NPARAM; np++)
    RELEASE_STRING(&PARAM[np]);*/

  RETURN->type = T_STRING;
  RETURN->_string.addr = (char *)string;
  RETURN->_string.start = 0;
  RETURN->_string.len = STRING_length(string);

  SUBR_LEAVE();
}



void SUBR_replace(void)
{
  char *ps;
  char *pp;
  char *pr;
  int ls, lp, lr;
  int is, pos;
  bool nocase = FALSE;

  SUBR_ENTER();

  SUBR_get_string_len(&PARAM[0], &ps, &ls);
  SUBR_get_string_len(&PARAM[1], &pp, &lp);
  SUBR_get_string_len(&PARAM[2], &pr, &lr);

  if (NPARAM == 4)
    nocase = SUBR_get_integer(&PARAM[3]) == GB_COMP_TEXT;

	if (lp >= lr)
		SUBST_init_max(ls);
	else
  	SUBST_init_ext(ls, (lr - lp) * 16);
  
  if (ls > 0 && lp > 0)
  {
		is = 0;
	
		for(;;)
		{
			pos = STRING_search(ps, ls, pp, lp, 1, FALSE, nocase);
			if (pos == 0)
				break;

			pos--;

			if (pos > 0)
				SUBST_add(ps, pos);

			SUBST_add(pr, lr);

			pos += lp;

			ps += pos;
			ls -= pos;

			if (ls <= 0)
				break;
		}
		
		SUBST_add(ps, ls);
  }

  SUBST_exit();

  RETURN->type = T_STRING;
  RETURN->_string.addr = SUBST_buffer;
  RETURN->_string.start = 0;
  RETURN->_string.len = STRING_length(RETURN->_string.addr);

  SUBR_LEAVE();
}


void SUBR_split(void)
{
  CARRAY *array;
  char *str;
  int lstr;
  char *sep = "";
  char *esc = "";
  bool no_void = FALSE;

  SUBR_ENTER();

  SUBR_get_string_len(&PARAM[0], &str, &lstr);
  
  if (NPARAM >= 2)
  {
    sep = SUBR_get_string(&PARAM[1]);
    if (NPARAM >= 3)
    {
      esc = SUBR_get_string(&PARAM[2]);
      if (NPARAM == 4)
      {
        VALUE_conv(&PARAM[3], T_BOOLEAN);
        no_void = PARAM[3]._boolean.value;
      }
    }
  }

  OBJECT_create((void **)(void *)&array, CLASS_StringArray, NULL, NULL, 0);

  if (lstr)
  {
    if (*sep) STRING_ref(sep);
    if (*esc) STRING_ref(esc);

    CARRAY_split(array, str, lstr, sep, esc, no_void);

    if (*sep) STRING_unref(&sep);
    if (*esc) STRING_unref(&esc);
  }

  RETURN->_object.class = CLASS_StringArray;
  RETURN->_object.object = array;

  SUBR_LEAVE();
}


void SUBR_scan(void)
{
  CARRAY *array;
  char *str;
  int len_str;
  char *pat;
  int len_pat;

  SUBR_ENTER_PARAM(2);

  SUBR_get_string_len(&PARAM[0], &str, &len_str);
  SUBR_get_string_len(&PARAM[1], &pat, &len_pat);

  OBJECT_create((void **)(void *)&array, CLASS_StringArray, NULL, NULL, 0);

  if (len_str && len_pat)
  	REGEXP_scan(array, pat, len_pat, str, len_str);

  RETURN->_object.class = CLASS_StringArray;
  RETURN->_object.object = array;

  SUBR_LEAVE();
}


void SUBR_iconv(void)
{
  char *str;
  const char *src;
  const char *dst;
  char *result;
  int len;

  SUBR_ENTER_PARAM(3);

  str = SUBR_get_string(&PARAM[0]);
  len = PARAM[0]._string.len;

  src = SUBR_get_string(&PARAM[1]);
  dst = SUBR_get_string(&PARAM[2]);

  STRING_conv(&result, str, len, src, dst, TRUE);

  if (!result)
    RETURN->type = T_NULL;
  else
  {
    RETURN->type = T_STRING;
    RETURN->_string.addr = result;
    RETURN->_string.start = 0;
    RETURN->_string.len = STRING_length(result);
  }

  SUBR_LEAVE();
}


void SUBR_sconv(void)
{
  char *str;
  const char *src;
  const char *dst;
  char *result;
  int len;

  SUBR_ENTER_PARAM(1);

  if (LOCAL_is_UTF8)
    return;

  str = SUBR_get_string(&PARAM[0]);
  len = PARAM[0]._string.len;

  if (EXEC_code & 0xF)
  {
    src = LOCAL_encoding;
    dst = "UTF-8";
  }
  else
  {
    src = "UTF-8";
    dst = LOCAL_encoding;
  }

  STRING_conv(&result, str, len, src, dst, TRUE);

  if (!result)
    RETURN->type = T_NULL;
  else
  {
    RETURN->type = T_STRING;
    RETURN->_string.addr = result;
    RETURN->_string.start = 0;
    RETURN->_string.len = STRING_length(result);
  }

  SUBR_LEAVE();
}

static int _is_ascii(int c)
{
  return (c & ~0x7F) == 0;
}

static int _is_letter(int c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int _is_lower(int c)
{
  return (c >= 'a' && c <= 'z');
}

static int _is_upper(int c)
{
  return (c >= 'A' && c <= 'Z');
}

static int _is_digit(int c)
{
  return (c >= '0' && c <= '9');
}

static int _is_hexa(int c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int _is_space(int c)
{
  return strchr(" \n\r\t\f\v", c) != NULL;
}

static int _is_blank(int c)
{
  return (c == 32 || c == '\t');
}

static int _is_punct(int c)
{
	return ((c > 32) && (c < 128) && !(_is_letter(c) || _is_digit(c)));
}


void SUBR_is_chr(void)
{
  static void *jump[] =
  {
    NULL, _is_ascii, _is_letter, _is_lower, _is_upper, _is_digit, _is_hexa, _is_space, _is_blank, _is_punct
  };

  char *addr;
  int len;
  int i;
  int (*func)(int);

  SUBR_ENTER_PARAM(1);

  VALUE_conv_string(PARAM);

  SUBR_get_string_len(PARAM, &addr, &len);

  func = jump[EXEC_code & 0x3F];

  for (i = 0; i < len; i++)
  {
    if (!(*func)(addr[i]))
      break;
  }

  RETURN->type = T_BOOLEAN;
  RETURN->_boolean.value = (len > 0 && i >= len) ? -1 : 0;

  SUBR_LEAVE();
}


