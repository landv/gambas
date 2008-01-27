/***************************************************************************

  gb_common_string.h

  Copyright (c) 2000-2007 Benoit Minisini <gambas@freesurf.fr>

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

#ifndef __GB_COMMON_STRING_H
#define __GB_COMMON_STRING_H

bool STRING_equal_same(const char *str1, const char *str2, int len);
bool STRING_equal_ignore_case(const char *str1, int len1, const char *str2, int len2);
int STRING_compare(const char *str1, int len1, const char *str2, int len2);
int STRING_compare_ignore_case(const char *str1, int len1, const char *str2, int len2);

//#define STRING_equal_same(_str1, _str2, _len) (memcmp(_str1, _str2, _len) == 0)
#define STRING_equal(_str1, _len1, _str2, _len2) ((_len1) == (_len2) && STRING_equal_same(_str1, _str2, _len1))

#endif
