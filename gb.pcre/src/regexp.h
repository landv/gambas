/***************************************************************************

  regexp.h

  (c) 2004 Rob Kudla <pcre-component@kudla.org>
  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __REGEXP_H
#define __REGEXP_H

#include "gambas.h"

#include "pcre.h"

#ifndef __REGEXP_C

extern GB_DESC CRegexpDesc[];
extern GB_DESC CRegexpSubmatchesDesc[];
extern GB_DESC CRegexpSubmatchDesc[];

#else

typedef
struct {
  GB_BASE ob;
  char *subject;
  char *pattern;
  int *ovector;
  int rc;
  int compiled;
  int eopts;
  int copts;
  pcre *code;
  char **smcache;
  int _submatch;
}
CREGEXP;

#define THIS  OBJECT(CREGEXP)

#endif

void REGEXP_init(void);
void REGEXP_exit(void);

#endif
