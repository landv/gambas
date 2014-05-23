/***************************************************************************

  gbx_regexp.c

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

#define __GBX_REGEXP_C

#include "gb_common.h"
#include "gb_common_case.h"

#include <ctype.h>

#include "gb_alloc.h"
#include "gb_array.h"
#include "gb_error.h"
#include "gbx_c_array.h"
#include "gbx_api.h"

#include "gbx_regexp.h"


static REGEXP_SCAN_FUNC _scan_cb = NULL;
static CARRAY *_scan_array;

bool REGEXP_match(const char *pattern, int len_pattern, const char *string, int len_string)
{
	unsigned char cp;
	unsigned char cs;

	#define _next_pattern() (cp = *pattern++, len_pattern--)
	#define _next_string(void) (cs = *string++, len_string--)

	/*if (len_pattern == 0 || len_string == 0)
		return FALSE;*/

	for(;;)
	{
		if (len_pattern == 0)
			return (len_string == 0);

		_next_pattern();

		if (cp == '*')
		{
			const char *p;

			if (len_pattern == 0)
			{
				if (_scan_cb)
					(*_scan_cb)(string, len_string);
				return TRUE;
			}

			p = string;

			for(;;)
			{
				if (REGEXP_match(pattern, len_pattern, string, len_string))
				{
					if (_scan_cb)
						(*_scan_cb)(p, string - p);
					return TRUE;
				}
				if (len_string == 0)
					return FALSE;
				_next_string();
			}
			return FALSE;
		}

		if (len_string == 0)
			return FALSE; /*end || (len_pattern == 0);*/

		_next_string();

		if (cp == '?')
			continue;

		if (cp == '[' && len_pattern > 0)
		{
			bool not = FALSE;
			bool in = FALSE;
			unsigned char cb = 0;

			_next_pattern();
			if (cp == '^')
			{
				not = TRUE;
				_next_pattern();
			}

			if (cp == cs)
			{
				in = TRUE;
				_next_pattern();
			}
			else
			{
				for(;;)
				{
					if (cp == '-' && len_pattern > 1 && cb && cb != '-')
					{
						_next_pattern();
						if (cb <= cs && cs <= cp)
						{
							in = TRUE;
							break;
						}
						cb = 0;
					}
					else if (cp == cs)
					{
						in = TRUE;
						break;
					}
					else
						cb = cp;

					_next_pattern();
					if (cp == ']')
						break;
				}
			}

			for(;;)
			{
				if (cp == ']')
					break;
				if (len_pattern == 0)
					THROW(E_REGEXP, "Missing ']'");
				_next_pattern();
			}

			if (in ^ not)
				continue;

			return FALSE;
		}
		
		if (cp == ' ')
		{
			if (cs > ' ')
				return FALSE;

			while (len_string && cs <= ' ')
				_next_string();

			if (cs > ' ')
			{
				string--;
				len_string++;
			}

			while (len_pattern && cp == ' ')
				_next_pattern();

			if (cp != ' ')
			{
				pattern--;
				len_pattern++;
			}

			continue;
		}

		if (cp == '{' && len_pattern > 0)
		{
			const char *save_string;
			int save_len_string;
			const char *save_pattern;
			int save_len_pattern;
			
			string--; len_string++;
			save_string = string;
			save_len_string = len_string;

		NEXT_SUB_PATTERN:

			for(;;)
			{
				_next_pattern();
				if (cp == ',' || cp == '}')
					break;
				_next_string();
				if (tolower(cp) != tolower(cs))
					break;
			}

			if (cp == ',' || cp == '}')
			{
				save_pattern = pattern - 1;
				save_len_pattern = len_pattern + 1;

				while (cp != '}')
				{
					if (len_pattern == 0)
						THROW(E_REGEXP, "Missing '}'");
					_next_pattern();
				}

				if (REGEXP_match(pattern, len_pattern, string, len_string))
					return TRUE;

				pattern = save_pattern;
				len_pattern = save_len_pattern;
				_next_pattern();
			}

			while (cp != ',')
			{
				if (cp == '}')
					return FALSE;

				_next_pattern();
			}

			string = save_string;
			len_string = save_len_string;

			goto NEXT_SUB_PATTERN;
		}

		if (cp == '\\')
		{
			if (len_pattern == 0)
				THROW(E_REGEXP, "Trailing backslash");
			_next_pattern();
		}

		if (tolower(cp) != tolower(cs))
			return FALSE;
	}
}


static void add_string(const char *str, int len)
{
	char **p = (char **)GB_ArrayAdd((GB_ARRAY)_scan_array);
	if (len)
		*p = STRING_new(str, len);
}


bool REGEXP_scan(CARRAY *array, const char *pattern, int len_pattern, const char *string, int len_string)
{
	bool match;

	_scan_cb = add_string;
	_scan_array = array;
	match = REGEXP_match(pattern, len_pattern, string, len_string);
	CARRAY_reverse(array, NULL);
	_scan_cb = NULL;
	_scan_array = NULL;

	return match;
}

