/***************************************************************************

  c_font.h

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

#ifndef __C_FONT_H
#define __C_FONT_H

#include "main.h"
typedef
	struct {
		GB_BASE ob;
		TTF_Font *font;
		char *name;
		int size;
		unsigned bold : 1;
		unsigned italic : 1;
		unsigned dirty : 1;
		SDL_Surface *surface;
		char *text;
		SDL_Color color;
	}
	CFONT;

#ifndef __C_FONT_C
extern GB_DESC FontDesc[];
#endif

SDL_Surface *FONT_render_text(CFONT *_object, char *text, SDL_Color color);

#endif /* __C_FONT_H */

