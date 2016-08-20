/***************************************************************************

  c_draw.h

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __C_DRAW_H
#define __C_DRAW_H

#include "main.h"
#include "c_font.h"

typedef
  struct {
		void *device;
		SDL_Renderer *renderer;
		CFONT *font;
		GB_COLOR color;
	}
	CDRAW;

#ifndef __C_DRAW_C

extern GB_DESC DrawDesc[];

void DRAW_begin(void *device);
void DRAW_end();

#endif
	
#endif /* __C_DRAW_H */

