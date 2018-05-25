/***************************************************************************

  gb_str.c

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

#define __GB_STR_C

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>

#include "gb_common.h"
#include "gb_error.h"
#include "gb_limit.h"
#include "gb_alloc.h"

#include "gb_str.h"

static char *_free_later = NULL;
static char *_last_str = NULL;
static int _last_len = 0;


void STR_vadd(char **str, const char *fmt, va_list args)
{
	va_list copy;
	int len, add;
	char *new;
	
	va_copy(copy, args);
	add = vsnprintf(NULL, 0, fmt, args);
	
	if (*str)
		len = (*str == _last_str ? _last_len : strlen(*str));
	else
		len = 0;
	
	ALLOC(&new, len + add + 1);
	if (*str) strcpy(new, *str);

	vsprintf(&new[len], fmt, copy);
	va_end(copy);
	
	*str = new;
	
	_last_str = new;
	_last_len = len + add;
}


void STR_add(char **str, const char *fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
	STR_vadd(str, fmt, args);
	va_end(args);
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


static char *str_add(char *d, const char *s)
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

  va_end(args);
  
  ALLOC(&cpy, len + 1);
  p = cpy;

  va_start(args, str);

  while (str)
  {
    p = str_add(p, str);
    str = va_arg(args, char *);
  }

  va_end(args);
  
  return cpy;
}


char *STR_upper(const char *str)
{
	char *s;
	char *p;
	
	p = s = STR_copy(str);
	while (*p)
	{
		*p = toupper(*p);
		p++;
	}
	
	return s;
}


char *STR_lower(const char *str)
{
	char *s;
	char *p;
	
	p = s = STR_copy(str);
	while (*p)
	{
		*p = tolower(*p);
		p++;
	}
	
	return s;
}


char *STR_free_later(char *str)
{
	if (_free_later)
		STR_free(_free_later);
	_free_later = str;
	return str;
}


char *STR_print(const char *fmt, ...)
{
	va_list args;
	char *str = NULL;
	
	va_start(args, fmt);
	STR_vadd(&str, fmt, args);
	va_end(args);
	return str;
}

