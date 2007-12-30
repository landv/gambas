/***************************************************************************

  gb_common_case_temp.h
  
  common useful routines

  (c) 2000-2005 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __COMMON_CASE_C

#include <ctype.h>
#include "gb_common.h"

PUBLIC char COMMON_tolower[256];
PUBLIC char COMMON_toupper[256];


PUBLIC void COMMON_case_init(void)
{
  int i;
  
  for (i = 0; i < 256; i++)
  {
    COMMON_tolower[i] = tolower(i);
    COMMON_toupper[i] = toupper(i);
  }
}


PUBLIC int COMMON_strcasecmp(const char *s1, const char *s2)
{
  register unsigned int i;
  register int d;
  register char c;
  
  for (i = 0;; i++)
  {
    c = COMMON_tolower[(unsigned char)s1[i]];
    d = c - COMMON_tolower[(unsigned char)s2[i]];
    if (d < 0)
      return -1;
    else if (d > 0)
      return 1;
    else if (c == 0)
      return 0;
  }
}

PUBLIC int COMMON_strncasecmp(const char *s1, const char *s2, size_t n)
{
  register unsigned int i;
  register int d;
  register char c;
  
  for (i = 0; i < n; i++)
  {
    c = COMMON_tolower[(unsigned char)s1[i]];
    d = c - COMMON_tolower[(unsigned char)s2[i]];
    if (d < 0)
      return -1;
    else if (d > 0)
      return 1;
  }
  
  return 0;
}


