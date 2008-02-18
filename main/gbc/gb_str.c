/***************************************************************************

  str.c

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

PUBLIC char *STR_add(char *d, const char *s)
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

PUBLIC char *STR_copy(const char *str)
{
  char *cpy;

  ALLOC(&cpy, strlen(str) + 1, "STR_copy");
  strcpy(cpy, str);
  return cpy;
}

PUBLIC void STR_free(const char *str)
{
  if (str)
    FREE(&str, "STR_free");
}


PUBLIC char *STR_cat(const char *str, ...)
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

  ALLOC(&cpy, len + 1, "STR_cat");
  p = cpy;

  va_start(args, str);

  while (str)
  {
    p = STR_add(p, str);
    str = va_arg(args, char *);
  }

  return cpy;
}


