/***************************************************************************

  regexp.c

  Regular expression management

  Copyright (C) 2000 Beno� Minisini <gambas@freesurf.fr>

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

PUBLIC bool REGEXP_match(const char *pattern, long len_pattern, const char *string, long len_string)
{
  unsigned char cp;
  unsigned char cs;

  void _next_pattern(void)
  {
    cp = *pattern;
    pattern++; len_pattern--;
  }

  void _next_string(void)
  {
    cs = *string;
    string++; len_string--;
  }

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
			continue;
    }

    if (cp == '[' && len_pattern > 0)
    {
      boolean not = FALSE;
      boolean in = FALSE;
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
          THROW(E_REGEXP, "Right square bracket missing");
        _next_pattern();
      }

      if (in ^ not)
        continue;

      return FALSE;
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


static void add_string(const char *str, long len)
{
	char **p = (char **)GB_ArrayAdd((GB_ARRAY)_scan_array);
	if (len)
		STRING_new(p, str, len);
}


PUBLIC bool REGEXP_scan(CARRAY *array, const char *pattern, long len_pattern, const char *string, long len_string)
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

