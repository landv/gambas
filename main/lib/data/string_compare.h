/***************************************************************************

  gb_common_string_temp.h

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

#include "gambas.h"

int STRING_compare(const char *str1, int len1, const char *str2, int len2)
{
	uint i;
	int len = len1 < len2 ? len1 : len2;
	int diff;
	register unsigned char c1, c2;

	for (i = 0; i < len; i++)
	{
		c1 = str1[i];
		c2 = str2[i];
		if (LIKELY(c1 > c2)) return 1;
		if (LIKELY(c1 < c2)) return -1;
	}

	diff = len1 - len2;
	return LIKELY(diff < 0) ? (-1) : LIKELY(diff > 0) ? 1 : 0;
}
