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

typedef
	struct {
		SDL_Surface *surface;
		SDL_Texture *texture;
		CWINDOW *window;
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

CIMAGE *IMAGE_create(SDL_Image *image);
SDL_Texture *IMAGE_get_texture(CIMAGE *_object, CWINDOW *window);
CIMAGE *IMAGE_create_from_window(CWINDOW *window, int x, int y, int w, int h);

SDL_Image *SDL_CreateImage(SDL_Surface *surface);
void SDL_FreeImage(SDL_Image *image);
SDL_Texture *SDL_GetTextureFromImage(SDL_Image *image, CWINDOW *window);

#endif /* __C_IMAGE_H */

