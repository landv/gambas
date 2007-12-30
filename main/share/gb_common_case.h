/***************************************************************************

  common_case.h

  Case conversion routines

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

#ifndef __GB_COMMON_CASE_H
#define __GB_COMMON_CASE_H

#include <ctype.h>
#include <string.h>

extern char COMMON_tolower[];
extern char COMMON_toupper[];

int COMMON_strcasecmp(const char *s1, const char *s2);
int COMMON_strncasecmp(const char *s1, const char *s2, long n);
int COMMON_case_init(void);

#ifdef tolower
#undef tolower
#endif
#ifdef toupper
#undef toupper
#endif
#ifdef strcasecmp
#undef strcasecmp
#endif
#ifdef strncasecmp
#undef strncasecmp
#endif

#define tolower(_c) (COMMON_tolower[(unsigned char)(_c)])
#define toupper(_c) (COMMON_toupper[(unsigned char)(_c)])
#define strcasecmp(_s1, _s2) (COMMON_strcasecmp((_s1), (_s2)))
#define strncasecmp(_s1, _s2, _n) (COMMON_strncasecmp((_s1), (_s2), (_n)))

#define NO_GAMBAS_CASE_REPLACEMENT

#endif
