/***************************************************************************

  Cdraw.h

  (c) 2006 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#ifndef __CDRAW_H
#define __CDRAW_H

#include "main.h"
#include "SDLgfx.h"
#include "Cfont.h"

typedef
	struct {
		void *device;
		SDLgfx *graphic;
		CFONT *font;
		Uint32 forecolor;
		Uint32 backcolor;
	}
	CDRAW;

#ifndef __CDRAW_CPP
extern GB_DESC CDraw[];
#endif /* __CDRAW_CPP */

void DRAW_begin(void *device);
void DRAW_end(void );

#endif /* __CDRAW_H */

