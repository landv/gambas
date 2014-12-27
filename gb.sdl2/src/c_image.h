/***************************************************************************

  c_image.h

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

#ifndef __C_IMAGE_H
#define __C_IMAGE_H

#include "main.h"
#include "c_window.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN

#define DEFAULT_IMAGE_FORMAT GB_IMAGE_ARGB
#define RMASK 0xFF000000
#define GMASK 0x00FF0000
#define BMASK 0x0000FF00
#define AMASK 0x000000FF

#else

#define DEFAULT_IMAGE_FORMAT GB_IMAGE_BGRA
#define RMASK 0x000000FF
#define GMASK 0x0000FF00
#define BMASK 0x00FF0000
#define AMASK 0xFF000000

#endif

typedef
	struct {
		SDL_Surface *surface;
		SDL_Texture *texture;
		int window_id;
	}
	SDL_Image;

typedef
	struct {
		GB_IMG img;
	}
	CIMAGE;

#ifndef __C_IMAGE_C
extern GB_DESC ImageDesc[];
#endif

SDL_Texture *IMAGE_get_texture(CIMAGE *_object, CWINDOW *window);

#endif /* __C_IMAGE_H */

