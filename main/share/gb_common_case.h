/***************************************************************************

  gb_common_case.h

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

#ifndef __GB_COMMON_CASE_H
#define __GB_COMMON_CASE_H

#ifdef __CYGWIN__
#include <strings.h>
#endif

#include <ctype.h>
#include <string.h>

extern unsigned char COMMON_tolower[];
extern unsigned char COMMON_toupper[];

int COMMON_strcasecmp(const char *s1, const char *s2);
int COMMON_strncasecmp(const char *s1, const char *s2, size_t n);

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
