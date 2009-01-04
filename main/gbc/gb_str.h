/***************************************************************************

  str.h

  Common string management routines

  (c) 2000-2005 Beno�t Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_STR_H
#define __GB_STR_H

#include "gb_alloc.h"

PUBLIC char *STR_copy(const char *str);
PUBLIC char *STR_cat(const char *str, ...);
PUBLIC char *STR_add(char *d, const char *s);

#define STR_free(_str) IFREE(_str, "STR_free")

#endif
