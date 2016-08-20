/***************************************************************************

  gb.draw.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __GB_DRAW_H
#define __GB_DRAW_H

#include "gb_common.h"
#include "gambas.h"

#define GB_DRAW_ALIGN_DEFAULT (-1)
#define GB_DRAW_COLOR_DEFAULT (-1)

enum {
  GB_DRAW_STATE_NORMAL = 0,
  GB_DRAW_STATE_DISABLED = 1,
	GB_DRAW_STATE_FOCUS = 2,
	GB_DRAW_STATE_HOVER = 4,
	GB_DRAW_STATE_ACTIVE = 8
  };

typedef
	void *GB_PICTURE;

typedef
	struct {
		int width;
		int height;
	}
	GB_PICTURE_INFO;

#ifndef __GB_IMAGE_DEFINED
#define __GB_IMAGE_DEFINED
typedef
	void *GB_IMAGE;
#endif
	
#ifndef __GB_COLOR_DEFINED
#define __GB_COLOR_DEFINED
typedef
	unsigned int GB_COLOR;
#endif

typedef
	void *GB_FONT;

#define DRAW_INTERFACE_VERSION 1

typedef
	struct {
		int version;
		struct {
			void *(*GetCurrent)();
			void (*Begin)(void *);
			void (*End)();
			bool (*IsPainted)(void *);
			}
			Paint;
		}
	DRAW_INTERFACE;

#define DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, width, height) \
	if (sw < 0) sw = width; \
	if (sh < 0) sh = height; \
	if (w < 0) w = sw; \
	if (h < 0) h = sh; \
  if (sx < 0) sw += sx, sx = 0; \
  if (sy < 0) sh += sy, sy = 0; \
  if (sw > ((width) - sx)) \
    sw = ((width) - sx); \
  if (sh > ((height) - sy)) \
    sh = ((height) - sy); \
  if (sx >= (width) || sy >= (height) || sw <= 0 || sh <= 0) \
    return;

#endif


 
