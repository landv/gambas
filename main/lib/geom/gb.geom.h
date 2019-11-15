/***************************************************************************

  gb.geom.h

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#ifndef __GB_GEOM_H
#define __GB_GEOM_H

#include "gambas.h"

enum
{
	ALIGN_NORMAL = 0x00,
	ALIGN_LEFT = 0x01,
	ALIGN_RIGHT = 0x02,
	ALIGN_CENTER = 0x03,
	ALIGN_TOP_NORMAL = 0x10,
	ALIGN_TOP_LEFT = 0x11,
	ALIGN_TOP_RIGHT = 0x12,
	ALIGN_TOP = 0x13,
	ALIGN_BOTTOM_NORMAL = 0x20,
	ALIGN_BOTTOM_LEFT = 0x21,
	ALIGN_BOTTOM_RIGHT = 0x22,
	ALIGN_BOTTOM = 0x23,
	ALIGN_JUSTIFY = 0x04,
};

#define ALIGN_IS_TOP(_align) (((_align) & 0xF0) == 0x10)
#define ALIGN_IS_BOTTOM(_align) (((_align) & 0xF0) == 0x20)
#define ALIGN_IS_MIDDLE(_align) (((_align) & 0xF0) == 0x00)
#define ALIGN_IS_LEFT(_align) (((_align) & 0xF) == 0x1 || (((_align) & 0xF) == 0x0 && !GB.System.IsRightToLeft()))
#define ALIGN_IS_RIGHT(_align) (((_align) & 0xF) == 0x2 || (((_align) & 0xF) == 0x0 && GB.System.IsRightToLeft()))
#define ALIGN_IS_CENTER(_align) (((_align) & 0xF) == 0x3)

typedef
	struct {
		GB_BASE ob;
		int x;
		int y;
		}
	GEOM_POINT;

typedef
	struct {
		GB_BASE ob;
		double x;
		double y;
		}
	GEOM_POINTF;

typedef
	struct {
		GB_BASE ob;
		int x;
		int y;
		int w;
		int h;
		}
	GEOM_RECT;

typedef
	struct {
		GB_BASE ob;
		double x;
		double y;
		double w;
		double h;
		}
	GEOM_RECTF;

#define GEOM_INTERFACE_VERSION 1

typedef
	struct {
		int version;
		GEOM_POINT *(*CreatePoint)(int x, int y);
		GEOM_POINTF *(*CreatePointF)(double x, double y);
		GEOM_RECT *(*CreateRect)(void);
		GEOM_RECTF *(*CreateRectF)(void);
		}
	GEOM_INTERFACE;

#endif


