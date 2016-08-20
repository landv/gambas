/***************************************************************************

  main.h

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

#ifndef __MAIN_H
#define __MAIN_H

#include "gambas.h"
#include "gb_common.h"
#include "gb_list.h"

#include "gb.geom.h"
#include "gb.image.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_opengl.h"

#ifndef __MAIN_C
extern GB_INTERFACE GB;
extern IMAGE_INTERFACE IMAGE;
extern GEOM_INTERFACE GEOM;

extern GB_CLASS CLASS_Window;
extern GB_CLASS CLASS_Image;
extern GB_CLASS CLASS_Font;
#endif

#define RAISE_ERROR(_msg) GB.Error(_msg ": &1", SDL_GetError());

#define SAME_COLORS(_col1, _col2) ((_col1)->r == (_col2)->r && (_col1)->g== (_col2)->g && (_col1)->b == (_col2)->b && (_col1)->a == (_col2)->a)

#define SDL_COLOR_TO_UINT(_color) ((_color)->b | ((_color)->g << 8) | ((_color)->r << 16) | ((_color)->a << 24))

#if SDL_BYTEORDER == SDL_BIG_ENDIAN

#define DEFAULT_IMAGE_FORMAT GB_IMAGE_ARGB
#define DEFAULT_SDL_IMAGE_FORMAT SDL_PIXELFORMAT_BGRA8888

#define BMASK 0xFF000000
#define GMASK 0x00FF0000
#define RMASK 0x0000FF00
#define AMASK 0x000000FF

#else

#define DEFAULT_IMAGE_FORMAT GB_IMAGE_BGRA
#define DEFAULT_SDL_IMAGE_FORMAT SDL_PIXELFORMAT_ARGB8888

#define BMASK 0x000000FF
#define GMASK 0x0000FF00
#define RMASK 0x00FF0000
#define AMASK 0xFF000000

#endif

#endif /* __MAIN_H */

