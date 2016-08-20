/***************************************************************************

  gb_common_string.h

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

#ifndef __GB_COMMON_STRING_H
#define __GB_COMMON_STRING_H

bool STRING_equal_same(const char *str1, const char *str2, int len);
bool STRING_equal_ignore_case_same(const char *str1, const char *str2, int len);
int STRING_compare(const char *str1, int len1, const char *str2, int len2);
int STRING_compare_ignore_case(const char *str1, int len1, const char *str2, int len2);

// valgrind says that STRING_equal_same() is globally faster than memcmp_sse4, so...
//#define STRING_equal_same(_str1, _str2, _len) (memcmp(_str1, _str2, _len) == 0)
#define STRING_equal(_str1, _len1, _str2, _len2) ((_len1) == (_len2) && STRING_equal_same(_str1, _str2, _len1))
#define STRING_equal_ignore_case(_str1, _len1, _str2, _len2) ((_len1) == (_len2) && STRING_equal_ignore_case_same(_str1, _str2, _len1))

#endif
