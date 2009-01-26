/***************************************************************************

  main.c

  gb.cairo component

  (c) 2009 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __MAIN_C

#include "main.h"
#include "c_cairo.h"

GB_INTERFACE GB EXPORT;
IMAGE_INTERFACE IMAGE EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CairoPatternDesc,
  CairoExtentsDesc,
  CairoDesc,
  NULL
};

int EXPORT GB_INIT(void)
{
  GB.GetInterface("gb.image", IMAGE_INTERFACE_VERSION, &IMAGE);
	return FALSE;
}

void EXPORT GB_EXIT()
{
}
