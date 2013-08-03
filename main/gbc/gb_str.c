/***************************************************************************

  gb_str.c

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

#define __GB_STR_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gb_alloc.h"

#include "gb_str.h"

char *STR_add(char *d, const char *s)
{
  for(;;)
  {
    if ((*d = *s) == 0)
      break;

    d++;
    s++;
  }

  return d;
}

char *STR_copy_len(const char *str, int len)
{
  char *cpy;

  ALLOC(&cpy, len  + 1);
  memcpy(cpy, str, len + 1);
  return cpy;
}

char *STR_copy(const char *str)
{
	return STR_copy_len(str, strlen(str));
}

char *STR_cat(const char *str, ...)
{
  va_list args;
  char *cpy;
  char *p;
  int len = 0;

  va_start(args, str);

  p = (char *)str;
  while (p)
  {
    len += strlen(p);
    p = va_arg(args, char *);
  }

  ALLOC(&cpy, len + 1);
  p = cpy;

  va_start(args, str);

  while (str)
  {
    p = STR_add(p, str);
    str = va_arg(args, char *);
  }

  va_end(args);
  
  return cpy;
}


