/***************************************************************************

  c_color.h

  The Color constants and functions

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

#ifndef __C_COLOR_H
#define __C_COLOR_H

#include "main.h"

#ifndef __C_COLOR_C
extern GB_DESC CColorDesc[];
extern GB_DESC CColorInfoDesc[];
#else
#define THIS_COLOR_INFO ((COLOR_INFO *)_object)
#endif

typedef
	struct {
		GB_BASE ob;
		int r, g, b, a;
		}
	COLOR_INFO;

void COLOR_rgb_to_hsv(int r, int g, int b, int *H, int *S, int *V);
void COLOR_hsv_to_rgb(int h, int s, int v, int *R, int *G, int *B);

#endif
