/***************************************************************************

  c_cairo.h

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

#ifndef __C_CAIRO_H
#define __C_CAIRO_H

#include "main.h"

#ifndef __C_CAIRO_C

extern GB_DESC CairoExtentsDesc[];
extern GB_DESC CairoTextExtentsDesc[];
extern GB_DESC CairoFontExtentsDesc[];
extern GB_DESC CairoPatternDesc[];
extern GB_DESC CairoMatrixDesc[];
extern GB_DESC CairoFontDesc[];
extern GB_DESC CairoDesc[];

#else

#define THIS_EXTENTS ((CAIRO_EXTENTS *)_object)
#define THIS_TEXT_EXTENTS ((CAIRO_TEXT_EXTENTS *)_object)
#define THIS_FONT_EXTENTS ((CAIRO_FONT_EXTENTS *)_object)
#define THIS_PATTERN ((CAIRO_PATTERN *)_object)
#define THIS_MATRIX ((CAIRO_MATRIX *)_object)

#endif

typedef
	struct {
		GB_BASE ob;
		double x1, y1, x2, y2;
		}
	CAIRO_EXTENTS;
	
typedef
	struct {
		GB_BASE ob;
		cairo_text_extents_t e;
		}
	CAIRO_TEXT_EXTENTS;
	
typedef
	struct {
		GB_BASE ob;
		cairo_font_extents_t e;
		}
	CAIRO_FONT_EXTENTS;
	
typedef
	struct {
		GB_BASE ob;
		cairo_pattern_t *pattern;
		void *ref;
	}
	CAIRO_PATTERN;

typedef
	struct {
		GB_BASE ob;
		cairo_matrix_t matrix;
	}
	CAIRO_MATRIX;

#endif