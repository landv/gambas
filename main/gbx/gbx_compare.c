/***************************************************************************

  compare.c

  Comparison functions

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

#define __GBX_COMPARE_C

#include "gb_common.h"
#include "gb_common_case.h"

#include <wctype.h>
#include <wchar.h>
#include <iconv.h>

#include "gbx_type.h"
#include "gbx_compare.h"
#include "gbx_date.h"
#include "gbx_object.h"
#include "gbx_class.h"
#include "gbx_exec.h"

static bool _descent = FALSE;


int compare_nothing(void *a, void *b)
{
  return 0;
}

int compare_integer(int *a, int *b)
{
  bool comp;

  if (*a < *b)
    comp = -1;
  else if (*a > *b)
    comp = 1;
  else
    return 0;

  if (_descent)
    comp = -comp;

  return comp;
}

int compare_short(short *a, short *b)
{
  bool comp;

  if (*a < *b)
    comp = -1;
  else if (*a > *b)
    comp = 1;
  else
    return 0;

  if (_descent)
    comp = -comp;

  return comp;
}


int compare_byte(unsigned char *a, unsigned char *b)
{
  bool comp;

  if (*a < *b)
    comp = -1;
  else if (*a > *b)
    comp = 1;
  else
    return 0;

  if (_descent)
    comp = -comp;

  return comp;
}


int compare_long(int64_t *a, int64_t *b)
{
  bool comp;

  if (*a < *b)
    comp = -1;
  else if (*a > *b)
    comp = 1;
  else
    return 0;

  if (_descent)
    comp = -comp;

  return comp;
}


int compare_float(double *a, double *b)
{
  bool comp;

  if (*a < *b)
    comp = -1;
  else if (*a > *b)
    comp = 1;
  else
    return 0;

  if (_descent)
    comp = -comp;

  return comp;
}


int compare_single(float *a, float *b)
{
  bool comp;

  if (*a < *b)
    comp = -1;
  else if (*a > *b)
    comp = 1;
  else
    return 0;

  if (_descent)
    comp = -comp;

  return comp;
}


int compare_date(DATE *a, DATE *b)
{
  bool comp;

  comp = DATE_comp(a, b);

  if (_descent)
    comp = -comp;

  return comp;
}

#define IMPLEMENT_COMPARE_STRING(_name, _func) \
int compare_string_##_name(char **pa, char **pb) \
{ \
  char *a; \
  char *b; \
  int comp; \
  \
  a = *pa; \
  if (!a) \
    a = ""; \
  \
  b = *pb; \
  if (!b) \
    b = ""; \
  \
  comp = _func(a, b); \
  if (_descent) \
    comp = -comp; \
  return comp; \
}

IMPLEMENT_COMPARE_STRING(binary, strcmp)
IMPLEMENT_COMPARE_STRING(text, strcasecmp)
//IMPLEMENT_COMPARE_STRING(lang, strcoll)

int COMPARE_string_lang(char *s1, int l1, char *s2, int l2, bool nocase, bool throw)
{
  char *charset;
  wchar_t *t1 = NULL;
  wchar_t *t2 = NULL;
  int i;
  
  if (l1 < 0)
  	l1 = s1 ? strlen(s1) : 0;
  	
	if (l2 < 0)
  	l2 = s2 ? strlen(s2) : 0;
  
	if (l1 == 0)
	{
		if (l2 == 0)
			return 0;
		else
			return (-1);
	}
	else if (l2 == 0)
		return 1;

	charset = EXEC_big_endian ? "UCS-4BE" : "UCS-4LE";

	if (STRING_conv((char **)(void *)&t1, s1, l1, "UTF-8", charset, throw)
		  || STRING_conv((char **)(void *)&t2, s2, l2, "UTF-8", charset, throw))
	{
		//fprintf(stderr, "warning: cannot convert UTF-8 string to %s for locale-aware comparison\n", charset);
		return nocase ? TABLE_compare_ignore_case(s1, l1, s2, l2) : TABLE_compare(s1, l1, s2, l2);
	}
	
	l1 = STRING_length((char *)t1) / sizeof(wchar_t);
	l2 = STRING_length((char *)t2) / sizeof(wchar_t);
	
	if (nocase)
	{
		for (i = 0; i < l1; i++)
			t1[i] = towlower(t1[i]);
		for (i = 0; i < l2; i++)
			t2[i] = towlower(t2[i]);
	}
	
	return wcscoll(t1, t2);
}

static int compare_string_lang(char **pa, char **pb)
{
	int diff = COMPARE_string_lang(*pa, -1, *pb, -1, FALSE, TRUE);
	return _descent ? (-diff) : diff;
}

static int compare_string_lang_case(char **pa, char **pb)
{
	int diff = COMPARE_string_lang(*pa, -1, *pb, -1, TRUE, TRUE);
	return _descent ? (-diff) : diff;
}

int COMPARE_object(void **a, void **b)
{
	bool comp;
	CLASS *ca = OBJECT_class(*a);
	CLASS *cb = OBJECT_class(*b);

	if (ca && ca->special[SPEC_COMPARE] != NO_SYMBOL)
	{
	  STACK_check(1);
		SP->_object.class = cb;
		SP->_object.object = *b;
		OBJECT_REF(*b, "compare_object");
		SP++;
		EXEC_special(SPEC_COMPARE, ca, *a, 1, FALSE);
		VALUE_conv(&SP[-1], T_INTEGER);
		SP--;
		comp = SP->_integer.value;
	}
	else if (cb && cb->special[SPEC_COMPARE] != NO_SYMBOL)
	{
	  STACK_check(1);
		SP->_object.class = ca;
		SP->_object.object = *a;
		OBJECT_REF(*a, "compare_object");
		SP++;
		EXEC_special(SPEC_COMPARE, cb, *b, 1, FALSE);
		VALUE_conv(&SP[-1], T_INTEGER);
		SP--;
		comp = (- SP->_integer.value);
	}
	else
	{
		comp = (*a == *b) ? 0 : (*a > *b) ? 1 : -1;
	}

	return _descent ? (-comp) : comp;
}

COMPARE_FUNC COMPARE_get(TYPE type, int mode)
{
  _descent = (mode & GB_COMP_DESCENT) != 0;
  mode &= GB_COMP_TYPE_MASK;

  switch(type)
  {
    case T_INTEGER:
      return (COMPARE_FUNC)compare_integer;

    case T_SHORT:
      return (COMPARE_FUNC)compare_short;

    case T_BYTE:
    case T_BOOLEAN:
      return (COMPARE_FUNC)compare_byte;

    case T_LONG:
      return (COMPARE_FUNC)compare_long;

    case T_FLOAT:
      return (COMPARE_FUNC)compare_float;

    case T_SINGLE:
      return (COMPARE_FUNC)compare_single;

    case T_DATE:
      return (COMPARE_FUNC)compare_date;

    case T_STRING:
      switch(mode)
      {
        case GB_COMP_LANG | GB_COMP_TEXT: return (COMPARE_FUNC)compare_string_lang_case;
        case GB_COMP_LANG: return (COMPARE_FUNC)compare_string_lang;
        case GB_COMP_TEXT: return (COMPARE_FUNC)compare_string_text;
        default: return (COMPARE_FUNC)compare_string_binary;
      }

    case T_OBJECT:
      return (COMPARE_FUNC)COMPARE_object;

    default:
      return (COMPARE_FUNC)compare_nothing;
  }
}
