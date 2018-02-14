/***************************************************************************

	gbx_compare.c

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
#include "gbx_regexp.h"
#include "gbx_c_string.h"


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

	return _descent ? (-comp) : comp;
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

	return _descent ? (-comp) : comp;
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

	return _descent ? (-comp) : comp;
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

	return _descent ? (-comp) : comp;
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

	return _descent ? (-comp) : comp;
}


int compare_date(DATE *a, DATE *b)
{
	bool comp;

	comp = DATE_comp(a, b);

	return _descent ? (-comp) : comp;
}

int COMPARE_string_lang(const char *s1, int l1, const char *s2, int l2, bool nocase, bool throw)
{
	wchar_t *t1 = NULL;
	wchar_t *t2 = NULL;
	int i, cmp;
	int lt1, lt2;

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

	if (STRING_convert_to_unicode(&t1, &lt1, s1, l1)
			|| STRING_convert_to_unicode(&t2, &lt2, s2, l2))
	{
		if (throw)
			THROW(E_CONV);
		else
			goto __FAILED;
	}

	if (nocase)
	{
		for (i = 0; i < lt1; i++)
			t1[i] = towlower(t1[i]);
		for (i = 0; i < lt2; i++)
			t2[i] = towlower(t2[i]);
	}

	errno = 0;
	cmp = wcscoll(t1, t2);
	if (!errno)
		return (cmp < 0) ? - 1 : (cmp > 0) ? 1 : 0;

__FAILED:

	return nocase ? TABLE_compare_ignore_case(s1, l1, s2, l2) : TABLE_compare(s1, l1, s2, l2);
}

/*
	Natural sort order.
	Based on the algorithm made by Martin Pol (http://sourcefrog.net/projects/natsort/)

	This software is copyright by Martin Pool, and made available under the same
	licence as zlib:

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from the use
	of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it freely,
	subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software in a
	product, an acknowledgment in the product documentation would be appreciated but
	is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.
*/

static int strnatcmp_compare_right(const char *a, int la, const char *b, int lb)
{
	int bias = 0;
	unsigned char ca, cb;

	/* The longest run of digits wins.  That aside, the greatest
	value wins, but we can't know that it will until we've scanned
	both numbers to know that they have the same magnitude, so we
	remember it in BIAS. */

	for (;; a++, b++, la--, lb--)
	{
		ca = (la > 0) ? *a : 0;
		cb = (lb > 0) ? *b : 0;

		if (!isdigit(ca) && !isdigit(cb))
			return bias;
		else if (!isdigit(ca))
			return -1;
		else if (!isdigit(cb))
			return +1;
		else if (ca < cb)
		{
			if (!bias)
				bias = -1;
		}
		else if (ca > cb)
		{
			if (!bias)
				bias = +1;
		}
		else if (!ca) // && !cb)
			return bias;
	}

	return 0;
}


static int strnatcmp_compare_left(const char *a, int la, const char *b, int lb)
{
	unsigned char ca, cb;

	/* Compare two left-aligned numbers: the first to have a
		different value wins. */
	for (;; a++, b++, la--, lb--)
	{
		ca = (la > 0) ? *a : 0;
		cb = (lb > 0) ? *b : 0;

		if (!isdigit(ca) && !isdigit(cb))
			return 0;
		else if (!isdigit(ca))
			return -1;
		else if (!isdigit(cb))
			return +1;
		else if (ca < cb)
			return -1;
		else if (ca > cb)
			return +1;
	}

	return 0;
}

int COMPARE_string_natural(const char *a, int la, const char *b, int lb, bool nocase)
{
	int ai, bi, lca, lcb;
	unsigned char ca, cb;
	int fractional, result;

	ai = bi = 0;

	for(;;)
	{
		for(;;)
		{
			if (ai >= la)
			{
				ca = 0;
				break;
			}
			ca = a[ai];
			if (ca > ' ')
				break;
			ai++;
		}

		for(;;)
		{
			if (bi >= lb)
			{
				cb = 0;
				break;
			}
			cb = b[bi];
			if (cb > ' ')
				break;
			bi++;
		}

		/* process run of digits */
		if (ca >= '0' && ca <= '9' && cb >= '0' && cb <= '9')
		{
			fractional = (ca == '0' || cb == '0');

			if (fractional)
			{
				if ((result = strnatcmp_compare_left(a+ai, la-ai, b+bi, lb-bi)) != 0)
					return result;
			}
			else
			{
				if ((result = strnatcmp_compare_right(a+ai, la-ai, b+bi, lb-bi)) != 0)
					return result;
			}
		}

		if (!ca)
		{
			if (!cb)
				return 0;
			else
				return -1;
		}
		else if (!cb)
			return 1;

		lca = STRING_utf8_get_char_length(ca);
		lcb = STRING_utf8_get_char_length(cb);
		if (lca > 1 || lcb > 1)
		{
			if ((result = COMPARE_string_lang(&a[ai], lca, &b[bi], lcb, nocase, FALSE)))
				return result;
			ai += lca;
			bi += lcb;
		}
		else
		{
			if (nocase)
			{
				ca = tolower(ca);
				cb = tolower(cb);
			}

			if (ca < cb)
				return -1;
			else if (ca > cb)
				return +1;
			++ai; ++bi;
		}
	}
}


/*#define IMPLEMENT_COMPARE_STRING(_name, _func) \
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
IMPLEMENT_COMPARE_STRING(case, strcasecmp)*/

static int compare_string_binary(char **pa, char **pb)
{
	int comp = TABLE_compare(*pa, STRING_length(*pa), *pb, STRING_length(*pb));
	return _descent ? -comp : comp;
}

static int compare_string_case(char **pa, char **pb)
{
	int comp = TABLE_compare_ignore_case(*pa, STRING_length(*pa), *pb, STRING_length(*pb));
	return _descent ? -comp : comp;
}

static int compare_string_lang(char **pa, char **pb)
{
	int comp = COMPARE_string_lang(*pa, STRING_length(*pa), *pb, STRING_length(*pb), FALSE, TRUE);
	return _descent ? (-comp) : comp;
}

static int compare_string_lang_case(char **pa, char **pb)
{
	int comp = COMPARE_string_lang(*pa, STRING_length(*pa), *pb, STRING_length(*pb), TRUE, TRUE);
	return _descent ? (-comp) : comp;
}

int COMPARE_string_like(const char *s1, int l1, const char *s2, int l2, bool nocase)
{
	int result;

	if (nocase)
	{
		if (REGEXP_match_pcre(s2, l2, s1, l1))
			return 0;
	}
	else
	{
		if (REGEXP_match(s2, l2, s1, l1))
			return 0;
	}
	result = TABLE_compare_ignore_case(s1, l1, s2, l2);
	return (result < 0) ? -1 : (result > 0) ? 1 : 0;
}

static int compare_string_like(char **pa, char **pb)
{
	int comp = COMPARE_string_like(*pa, STRING_length(*pa), *pb, STRING_length(*pb), FALSE);
	return _descent ? (-comp) : comp;
}

static int compare_string_match(char **pa, char **pb)
{
	int comp = COMPARE_string_like(*pa, STRING_length(*pa), *pb, STRING_length(*pb), TRUE);
	return _descent ? (-comp) : comp;
}

/*#define IMPLEMENT_COMPARE_STRING_CASE(_name, _nocase), _func \
static int compare_string_##_name(char **pa, char **pb) \
{ \
	int la = *pa ? strlen(*pa) : 0; \
	int lb = *pb ? strlen(*pb) : 0; \
	int diff = _func(*pa, la, *pb, lb, _nocase); \
	if (_descent) \
		return (-diff); \
	else \
		return diff; \
}

IMPLEMENT_COMPARE_STRING_CASE(like, FALSE, COMPARE_string_like)
IMPLEMENT_COMPARE_STRING_CASE(match, TRUE, COMPARE_string_like)
IMPLEMENT_COMPARE_STRING_CASE(natural, FALSE, COMPARE_string_natural)
IMPLEMENT_COMPARE_STRING_CASE(natural_case, TRUE, COMPARE_string_natural)*/

static int compare_string_natural(char **pa, char **pb)
{
	int comp = COMPARE_string_natural(*pa, STRING_length(*pa), *pb, STRING_length(*pb), FALSE);
	return _descent ? (-comp) : comp;
}

static int compare_string_natural_case(char **pa, char **pb)
{
	int comp = COMPARE_string_natural(*pa, STRING_length(*pa), *pb, STRING_length(*pb), TRUE);
	return _descent ? (-comp) : comp;
}


int COMPARE_object(void **a, void **b)
{
	bool comp;
	bool desc = _descent;
	CLASS *ca, *cb;

	/*{
		STACK_BACKTRACE *bt = STACK_get_backtrace();
		fprintf(stderr, "COMPARE_object\n");
		DEBUG_print_backtrace(bt);
		STACK_free_backtrace(&bt);
	}*/

	ca = OBJECT_class_null(*a);
	cb = OBJECT_class_null(*b);

	if (ca && cb)
	{
		if (ca->has_operators && CLASS_has_operator(ca, CO_COMP) && ca == cb)
		{
			void *func = ca->operators[CO_COMP];
			comp = (*(int (*)(void *, void *))func)(*a, *b);
			goto __RETURN;
		}

		if (ca->special[SPEC_COMPARE] != NO_SYMBOL)
		{
			STACK_check(1);
			SP->_object.class = cb;
			SP->_object.object = *b;
			OBJECT_REF(*b);
			SP++;
			EXEC_special(SPEC_COMPARE, ca, *a, 1, FALSE);
			VALUE_conv_integer(&SP[-1]);
			SP--;
			comp = SP->_integer.value;
			goto __RETURN;
		}

		if (cb->special[SPEC_COMPARE] != NO_SYMBOL)
		{
			STACK_check(1);
			SP->_object.class = ca;
			SP->_object.object = *a;
			OBJECT_REF(*a);
			SP++;
			EXEC_special(SPEC_COMPARE, cb, *b, 1, FALSE);
			VALUE_conv_integer(&SP[-1]);
			SP--;
			comp = (- SP->_integer.value);
			goto __RETURN;
		}
		
		_descent = desc;
	}

	comp = (*a == *b) ? 0 : (*a > *b) ? 1 : -1;

__RETURN:

	return desc ? (-comp) : comp;
}

int COMPARE_variant(VARIANT *a, VARIANT *b)
{
	TYPE type;
	VALUE value;
	int comp;

	if (a->type == T_NULL)
		return b->type == T_NULL ? 0 : -1;
	else if (b->type == T_NULL)
		return 1;

	if (TYPE_is_object(a->type))
	{
		if (TYPE_is_object(b->type))
			return COMPARE_object(&a->value._object, &b->value._object);
		else
			return 1;
	}
	else if (TYPE_is_object(b->type))
		return -1;

	if (a->type == b->type)
		return (*COMPARE_get_func(a->type, 0))(&a->value, &b->value);

	type = Max(a->type, b->type);

	if (b->type == type)
	{
		VARIANT *c;

		c = a;
		a = b;
		b = c;
		_descent = !_descent;
	}

	value.type = T_VARIANT;
	value._variant.vtype = b->type;
	value._variant.value.data = b->value.data;

	BORROW(&value);
	VALUE_conv(&value, type);
	VALUE_conv_variant(&value);
	comp = (*COMPARE_get_func(type, 0))(&a->value, &value._variant.value);
	RELEASE(&value);

	return comp;
}

static COMPARE_FUNC _string_func[] = {
	/*  0 */ compare_string_binary,
	/*  1 */ compare_string_case,
	/*  2 */ compare_string_lang,
	/*  3 */ compare_string_lang_case,
	/*  4 */ compare_string_like,
	/*  5 */ compare_string_match,
	/*  6 */ compare_string_like,
	/*  7 */ compare_string_match,
	/*  8 */ compare_string_natural,
	/*  9 */ compare_string_natural_case,
	/* 10 */ compare_string_natural,
	/* 11 */ compare_string_natural_case,
	/* 12 */ compare_string_natural,
	/* 13 */ compare_string_natural_case,
	/* 14 */ compare_string_natural,
	/* 15 */ compare_string_natural_case,
};


COMPARE_FUNC COMPARE_get_func(TYPE type, int mode)
{
	_descent = (mode & GB_COMP_DESCENT) != 0;
	mode &= GB_COMP_TYPE_MASK;

	if (type >= T_OBJECT)
		return (COMPARE_FUNC)COMPARE_object;

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
			return _string_func[mode];

		case T_POINTER:
			#ifdef OS_64BITS
				return (COMPARE_FUNC)compare_long;
			#else
				return (COMPARE_FUNC)compare_integer;
			#endif

		case T_VARIANT:
			return (COMPARE_FUNC)COMPARE_variant;

		default:
			return (COMPARE_FUNC)compare_nothing;
	}
}

COMPARE_STRING_FUNC COMPARE_get_string_func(int mode)
{
	mode &= GB_COMP_TYPE_MASK;
	
	if (mode == GB_COMP_BINARY)
		return (COMPARE_STRING_FUNC)STRING_compare;
	else if (mode == GB_COMP_NOCASE)
		return (COMPARE_STRING_FUNC)STRING_compare_ignore_case;
	else
	{
		if (mode & GB_COMP_NATURAL)
			return (COMPARE_STRING_FUNC)COMPARE_string_natural;
		else if (mode & GB_COMP_LIKE)
			return (COMPARE_STRING_FUNC)COMPARE_string_like;
		else if (mode & GB_COMP_LANG)
			return (COMPARE_STRING_FUNC)COMPARE_string_lang;
		else
			THROW(E_ARG);
	}
}
