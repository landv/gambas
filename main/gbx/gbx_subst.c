/***************************************************************************

  subst.c

  string substitution routines

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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


static char *subst_buffer = NULL;
static char _temp[16];
static int _ntemp;

static void dump_temp(void)
{
	int n = _ntemp;

	if (n)
	{
		_ntemp = 0;
		SUBST_add(_temp, n);
	}
}

PUBLIC void SUBST_init(void)
{
  subst_buffer = NULL;
  _ntemp = 0;
}

// len == 0 est possible ! On peut vouloir ajouter une chaîne vide.

PUBLIC void SUBST_add(const char *src, long len)
{
  long old_len;

	if (!src)
		return;

	if (len < 0)
		len = strlen(src);

  if (len <= 0)
    return;

	dump_temp();

  old_len = STRING_length(subst_buffer);

  STRING_extend(&subst_buffer, old_len + len);
  memcpy(&subst_buffer[old_len], src, len);
}

PUBLIC void SUBST_add_char(unsigned char c)
{
	if (_ntemp == sizeof(_temp))
		dump_temp();
	_temp[_ntemp++] = c;
}


PUBLIC void SUBST_exit(void)
{
	dump_temp();
  STRING_extend_end(&subst_buffer);
}

PUBLIC char *SUBST_buffer(void)
{
  return subst_buffer;
}


