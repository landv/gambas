/***************************************************************************

  main.c

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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
#include <ctype.h>

#include "gb_common.h"
#include "cpoint.h"
#include "crect.h"
#include "cpaint.h"
#include "main.h"


GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;

GB_DESC *GB_CLASSES [] EXPORT =
{
	PointDesc,
	PointFDesc,
	RectDesc,
	RectFDesc,
	//CDrawClipDesc,
	//CDrawStyleDesc,
	//CDrawDesc,
	PaintExtentsDesc,
	PaintMatrixDesc,
	PaintBrushDesc,
	PaintDesc,
  NULL
};

void *GB_DRAW_1[] EXPORT =
{
	(void *)1,
	(void *)PAINT_get_current,
	(void *)PAINT_begin,
	(void *)PAINT_end,
	NULL
};

int EXPORT GB_INIT(void)
{
	GB.Component.Load("gb.image");
  GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
  return 0;
}


void EXPORT GB_EXIT()
{
}

