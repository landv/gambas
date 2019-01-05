/***************************************************************************

  gb_str.h

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

#ifndef __GB_STR_H
#define __GB_STR_H

#include <stdarg.h>

char *STR_copy(const char *str);
char *STR_copy_len(const char *str, int len);
char *STR_cat(const char *str, ...);
char *STR_upper(const char *str);
char *STR_lower(const char *str);

void STR_free(char *str);
char *STR_free_later(char *str);

void STR_add(char **str, const char *fmt, ...);
char *STR_print(const char *fmt, ...);
void STR_vadd(char **str, const char *fmt, va_list args);

#endif
