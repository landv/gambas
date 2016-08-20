/***************************************************************************

  gb_str.h

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

#ifndef __GB_STR_H
#define __GB_STR_H

#include "gb_alloc.h"

char *STR_copy(const char *str);
char *STR_copy_len(const char *str, int len);
char *STR_cat(const char *str, ...);
char *STR_add(char *d, const char *s);

#define STR_free(_str) IFREE(_str)

#endif
