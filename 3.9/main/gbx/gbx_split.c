/***************************************************************************

  gbx_split.c

  (c) 2000-2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GBX_SPLIT_C

#include "gb_common.h"
#include "gb_error.h"
#include "gb_array.h"
#include "gbx_c_array.h"

static CARRAY *_array;
static bool _novoid;
static char *_entry;
static const char *_ptr;
static int _lptr;

static void add_char_real(const char *p)
{
	if (_lptr)
	{
		int old_len = STRING_length(_entry);

		_entry = STRING_extend(_entry, old_len + _lptr);
		memcpy(&_entry[old_len], _ptr, _lptr);
		_entry[old_len + _lptr] = 0;
	}

	_ptr = p;
	_lptr = p ? 1 : 0;
}

#define add_char(_p) \
({ \
	if ((_p) && (_p) == (_ptr + _lptr)) \
		_lptr++; \
	else \
		add_char_real(_p); \
})

static void add_entry()
{
	add_char_real(NULL);
	
	if (!_entry)
	{
		if (!_novoid)
			ARRAY_add_void((char ***)&_array->data);
	}
	else
	{
		*((char **)ARRAY_add((char ***)&_array->data)) = _entry;
		_entry = NULL;
	}
	
	//fprintf(stderr, "** add_entry\n");
}

static void split_fast(CARRAY *array, const char *str, int lstr, const char *sep, int lsep, bool no_void)
{
	const char *ptr = NULL;
	int lptr = 0;

	#define add_entry_fast() \
	({ \
		if (lptr) \
		{ \
			*((char **)ARRAY_add((char ***)&array->data)) = STRING_new(ptr, lptr); \
			lptr = 0; \
		} \
		else if (!no_void) \
		{ \
			ARRAY_add_void((char ***)&array->data); \
		} \
	})

	if (lsep == 1)
	{
		char csep = sep[0];

		while (lstr--)
		{
			if (*str == csep)
			{
				add_entry_fast();
			}
			else
			{
				if (!lptr) ptr = str;
				lptr++;
			}

			str++;
		}
	}
	else
	{
		while (lstr--)
		{
			if (memchr(sep, *str, lsep))
			{
				add_entry_fast();
			}
			else
			{
				if (!lptr) ptr = str;
				lptr++;
			}

			str++;
		}
	}

	add_entry_fast();
}

CARRAY *STRING_split(const char *str, int lstr, const char *sep, int lsep, const char *esc, int lesc, bool no_void, bool keep_esc)
{
	CARRAY *array;
	int i;
	char c;
	bool escape;
	char escl, escr;

	array = OBJECT_create(CLASS_StringArray, NULL, NULL, 0);
	if (lstr == 0)
		return array;

	if (sep == NULL || lsep == 0)
	{
		sep = ",";
		lsep = 1;
	}

	if (esc == NULL || lesc == 0)
	{
		split_fast(array, str, lstr, sep, lsep, no_void);
	}
	else
	{
		_array = array;
		_entry = NULL;
		_novoid = no_void;
		_ptr = NULL;
		_lptr = 0;

		escl = esc[0];
		if (lesc >= 2)
			escr = esc[1];
		else
			escr = escl;

		if (escr == *sep)
		{
			for (i = 0; i < lstr; i++)
			{
				c = *str;

				if (c == escl)
				{
					i++;
					str++;
					if (i < lstr)
						add_char(str);
				}
				else if (c == *sep || (lsep > 1 && memchr(&sep[1], c, lsep - 1)))
				{
					add_entry();
				}
				else
					add_char(str);

				str++;
			}
		}
		else
		{
			escape = FALSE;

			for (i = 0; i < lstr; i++)
			{
				c = *str;

				if (escape)
				{
					if (c != escr)
						add_char(str);
					else if ((i < (lstr - 1)) && str[1] == escr)
					{
						add_char(str);
						str++;
						i++;
					}
					else
					{
						escape = FALSE;
						if (keep_esc)
							add_char(str);
					}
				}
				else if (c == escl)
				{
					escape = TRUE;
					if (keep_esc)
						add_char(str);
				}
				else if (c == *sep || (lsep > 1 && memchr(&sep[1], c, lsep - 1)))
				{
					add_entry();
				}
				else
					add_char(str);

				str++;
			}
		}
		
		add_entry();
	}
	
	array->count = ARRAY_count(array->data);

	return array;
}
