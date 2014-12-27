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
#include "SDL_opengl.h"

#ifndef __MAIN_C
extern GB_INTERFACE GB;
extern IMAGE_INTERFACE IMAGE;
extern GEOM_INTERFACE GEOM;

extern GB_CLASS CLASS_Window;
extern GB_CLASS CLASS_Image;
#endif

#define RAISE_ERROR(_msg) GB.Error(_msg ": &1", SDL_GetError());

#endif /* __MAIN_H */

