/***************************************************************************

  main.c

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "regexp.h"

#include "main.h"

GB_INTERFACE *GB_PTR EXPORT;

void *GB_PCRE_1[] EXPORT = {

  (void *)PCRE_INTERFACE_VERSION,
  (void *)REGEXP_match,
  NULL
  };

GB_DESC *GB_CLASSES[] EXPORT =
{
  CRegexpDesc,
  CRegexpSubmatchesDesc,
  CRegexpSubmatchDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
  return 0;
}


void EXPORT GB_EXIT()
{
}

