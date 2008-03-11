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

void SUBST_dump_temp(void)
{
	int n = SUBST_ntemp;

	if (n)
	{
		SUBST_ntemp = 0;
		SUBST_add(SUBST_temp, n);
	}
}

void SUBST_init(void)
{
  SUBST_buffer = NULL;
  SUBST_ntemp = 0;
}

// len == 0 est possible ! On peut vouloir ajouter une chaîne vide.

void SUBST_add(const char *src, int len)
{
  int old_len;

	if (!src)
		return;

	if (len < 0)
		len = strlen(src);

  if (len <= 0)
    return;

	SUBST_dump_temp();

  old_len = STRING_length(SUBST_buffer);

  STRING_extend(&SUBST_buffer, old_len + len);
  memcpy(&SUBST_buffer[old_len], src, len);
}


void SUBST_exit(void)
{
	SUBST_dump_temp();
  STRING_extend_end(&SUBST_buffer);
}
