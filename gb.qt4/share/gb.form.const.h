/***************************************************************************

  gb.form.const.h

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

#ifndef __GB_FORM_H
#define __GB_FORM_H

#include "gb.geom.h"

#define CONST_MAGIC 0x12345678

enum {
	BORDER_NONE = 0,
	BORDER_PLAIN = 1,
	BORDER_SUNKEN = 2,
	BORDER_RAISED = 3,
	BORDER_ETCHED = 4
	};

enum {
	ARRANGE_NONE = 0,
	ARRANGE_HORIZONTAL = 1,
	ARRANGE_VERTICAL = 2,
	ARRANGE_ROW = 3,
	ARRANGE_LEFT_RIGHT = 3,
	ARRANGE_COLUMN = 4,
	ARRANGE_TOP_BOTTOM = 4,
	ARRANGE_FILL = 5,
	ARRANGE_TABLE = 6
	};

enum
{
	SELECT_NONE = 0,
	SELECT_SINGLE = 1,
	SELECT_MULTIPLE = 2
};

enum
{
	LINE_NONE = 0, //Qt::NoPen,
	LINE_SOLID = 1, //Qt::SolidLine,
	LINE_DASH = 2, //Qt::DashLine,
	LINE_DOT = 3, //Qt::DotLine,
	LINE_DASH_DOT = 4, //Qt::DashDotLine,
	LINE_DASH_DOT_DOT = 5, //Qt::DashDotDotLine
};

enum
{
	FILL_NONE = 0, //Qt::NoBrush,
	FILL_SOLID = 1, //Qt::SolidPattern,
	FILL_DENSE_94 = 2, //Qt::Dense1Pattern,
	FILL_DENSE_88 = 3, //Qt::Dense2Pattern,
	FILL_DENSE_63 = 4, //Qt::Dense3Pattern,
	FILL_DENSE_50 = 5, //Qt::Dense4Pattern,
	FILL_DENSE_37 = 6, //Qt::Dense5Pattern,
	FILL_DENSE_12 = 7, //Qt::Dense6Pattern,
	FILL_DENSE_06 = 8, //Qt::Dense7Pattern,
	FILL_HORIZONTAL = 9, //Qt::HorPattern,
	FILL_VERTICAL = 10, //Qt::VerPattern,
	FILL_CROSS = 11, //Qt::CrossPattern,
	FILL_DIAGONAL = 12, //Qt::BDiagPattern,
	FILL_BACK_DIAGONAL = 13, //Qt::FDiagPattern,
	FILL_CROSS_DIAGONAL = 14, //Qt::DiagCrossPattern
};

enum {
	SCROLL_NONE = 0,
	SCROLL_HORIZONTAL = 1,
	SCROLL_VERTICAL = 2,
	SCROLL_BOTH = 3
	};

enum {
	POINTER_MOUSE = 0,
	POINTER_PEN = 1,
	POINTER_ERASER = 2,
	POINTER_CURSOR = 3
};

enum {
	MOUSE_LEFT = 1,
	MOUSE_MIDDLE = 2,
	MOUSE_RIGHT = 4,
	MOUSE_SHIFT = 256,
	MOUSE_CTRL = 512,
	MOUSE_ALT = 1024,
	MOUSE_META = 2048
};


#endif
