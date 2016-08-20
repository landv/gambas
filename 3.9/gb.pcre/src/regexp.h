/***************************************************************************

  regexp.h

  (c) 2004 Rob Kudla <pcre-component@kudla.org>
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
	struct 
	{
		GB_BASE ob;
		char *subject;
		char *pattern;
		int *ovector;
		int ovecsize;
		int count;
		int eopts;
		int copts;
		pcre *code;
		int _submatch;
		int error;
	}
	CREGEXP;

#define THIS OBJECT(CREGEXP)

#endif

bool REGEXP_match(const char *subject, int lsubject, const char *pattern, int lpattern, int coptions, int eoptions);

#endif
