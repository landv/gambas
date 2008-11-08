/***************************************************************************

  subst.c

  string substitution routines

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

#define __GBX_SUBST_C

#include "gb_common.h"
#include "gb_error.h"
#include "gbx_value.h"
#include "gbx_debug.h"

#include "gbx_string.h"
#include "gbx_subst.h"

char *SUBST_buffer = NULL;
char SUBST_temp[SUBST_TEMP_SIZE];
int SUBST_ntemp;

static int _inc = 0;
static int _len = 0;
static int _max = 0;
static char *_ptr;

void SUBST_dump_temp(void)
{
	int n = SUBST_ntemp;

	if (n)
	{
		SUBST_ntemp = 0;
		SUBST_add(SUBST_temp, n);
	}
}

void SUBST_init_ext(int len, int inc)
{
	if (inc == 0)
		inc = 32;
	if (len == 0)
		len = inc;
		
	STRING_new(&SUBST_buffer, NULL, len);
	_inc = inc;
	_max = len;
	_len = 0;
	_ptr = SUBST_buffer;
  SUBST_ntemp = 0;
}

// len == 0 est possible ! On peut vouloir ajouter une chaîne vide.

void SUBST_add(const char *src, int len)
{
  int pos;

	if (!src)
		return;

	if (len < 0)
		len = strlen(src);

  if (len <= 0)
    return;

	SUBST_dump_temp();

	_len += len;

	if (_len >= _max)
	{
		pos = (_len - _max) * 4;
		if (pos < _inc)
			pos = _inc;
		else
		{
			_inc = (pos + 31) & ~31;
			if (_inc > 1024)
				_inc = 1024;
		}
		
		_max += pos;
		
		//fprintf(stderr, "STRING_extend: %d\n", _max - STRING_length(SUBST_buffer));
		pos = _ptr - SUBST_buffer;
		STRING_extend(&SUBST_buffer, _max);
		_ptr = &SUBST_buffer[pos];
	}

	memcpy(_ptr, src, len);
	_ptr += len;
}


void SUBST_exit(void)
{
	SUBST_dump_temp();
	STRING_extend(&SUBST_buffer, _len);
  STRING_extend_end(&SUBST_buffer);
}
