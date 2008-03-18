/***************************************************************************

  main.c

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __MAIN_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "gb_common.h"

#include "CDraw.h"
#include "main.h"


GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES [] EXPORT =
{
	CDrawClipDesc,
	CDrawStyleDesc,
	CDrawDesc,
  NULL
};

void *GB_DRAW_1[] EXPORT =
{
	(void *)1,
	(void *)DRAW_get_current,
	(void *)DRAW_begin,
	(void *)DRAW_end,
	NULL
};

int EXPORT GB_INIT(void)
{
  return 0;
}


void EXPORT GB_EXIT()
{
}

