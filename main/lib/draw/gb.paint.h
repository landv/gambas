/***************************************************************************

  gb.paint.h

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#ifndef __GB_PAINT_H
#define __GB_PAINT_H

#include "gb_common.h"
#include "gambas.h"
#include "gb.draw.h"

enum {
	GB_PAINT_EXTEND_PAD,
	GB_PAINT_EXTEND_REPEAT,
	GB_PAINT_EXTEND_REFLECT
};

enum {
	GB_PAINT_FILL_RULE_WINDING,
	GB_PAINT_FILL_RULE_EVEN_ODD
};

enum {
	GB_PAINT_LINE_CAP_BUTT,
	GB_PAINT_LINE_CAP_ROUND,
	GB_PAINT_LINE_CAP_SQUARE
};

enum {
	GB_PAINT_LINE_JOIN_MITER,
	GB_PAINT_LINE_JOIN_ROUND,
	GB_PAINT_LINE_JOIN_BEVEL
};

enum {
	GB_PAINT_OPERATOR_CLEAR,
	GB_PAINT_OPERATOR_SOURCE,
	GB_PAINT_OPERATOR_OVER,
	GB_PAINT_OPERATOR_IN,
	GB_PAINT_OPERATOR_OUT,
	GB_PAINT_OPERATOR_ATOP,
	GB_PAINT_OPERATOR_DEST,
	GB_PAINT_OPERATOR_DEST_OVER,
	GB_PAINT_OPERATOR_DEST_IN,
	GB_PAINT_OPERATOR_DEST_OUT
	GB_PAINT_OPERATOR_DEST_ATOP,
	GB_PAINT_OPERATOR_XOR,
	GB_PAINT_OPERATOR_ADD,
	GB_PAINT_OPERATOR_SATURATE
};

typedef
	struct {
		double x1, x2, y1, y2;
	}
	GB_EXTENTS;

typedef
	void *GB_BRUSH;
	
typedef
	void *GB_TRANSFORM;
	
typedef
	struct {
		GB_BASE ob;
		GB_EXTENTS ext;
	}
	PAINT_EXTENTS;
	
typedef
	struct {
		GB_BASE ob;
		GB_TRANSFORM matrix;
	}
	PAINT_MATRIX;
	
typedef
	struct {
		GB_BASE ob;
		GB_BRUSH brush;
	}
	PAINT_BRUSH;

typedef
	struct GB_PAINT {
		struct GB_PAINT_DESC *desc;         // drawing driver
		struct GB_PAINT *previous;          // previous drawing context
		void *device;                      // drawing object
		int width;                         // device width in device coordinates
		int height;                        // device height in device coordinates
		int resolution;                    // device resolution in DPI
		char extra[0];                     // driver-specific state
	}
	PACKED
	GB_PAINT;

typedef
	struct GB_PAINT_DESC {
		// Size of the GB_PAINT structure extra data
		int size;
		// Begins and terminates the drawing
		int (*Begin)(GB_PAINT *d);
		void (*End)(GB_PAINT *d);
		
		void (*Save)(GB_PAINT *d);
		void (*Restore)(GB_PAINT *d);
		
		void (*Font)(GB_PAINT *d, bool set, GB_FONT *font);
		
		void (*Clip)(GB_PAINT *d, bool preserve);
		void (*ResetClip)(GB_PAINT *d);
		void (*ClipExtents)(GB_PAINT *d, GB_EXTENTS *ext);
	
		void (*Fill)(GB_PAINT *d, bool preserve);
		void (*Stroke)(GB_PAINT *d, bool preserve);
		
		void (*PathExtents)(GB_PAINT *d, GB_EXTENTS *ext);
		bool (*PathContains)(GB_PAINT *d, double x, double y);
		
		void (*Dash)(GB_PAINT *d, bool set, double **dash, int *count);
		void (*DashOffset)(GB_PAINT *d, bool set, double *offset);
		
		void (*FillRule)(GB_PAINT *d, bool set, int *value);
		void (*LineCap)(GB_PAINT *d, bool set, int *value);
		void (*LineJoin)(GB_PAINT *d, bool set, int *value);
		void (*LineWidth)(GB_PAINT *d, bool set, double *value);
		void (*MiterLimit)(GB_PAINT *d, bool set, double *value);
		
		void (*Operator)(GB_PAINT *d, bool set, int *value);

		void (*NewPath)(GB_PAINT *d);
		void (*ClosePath)(GB_PAINT *d);
		
		void (*Arc)(GB_PAINT *d, double xc, double yc, double radius, double a1, double a2);
		void (*Rectangle)(GB_PAINT *d, double x, double y, double width, double height);
		void (*MoveTo)(GB_PAINT *d, double x, double y);
		void (*LineTo)(GB_PAINT *d, double x, double y);
		void (*CurveTo)(GB_PAINT *d, double x1, double y1, double x2, double y2, double x3, double y3);
	
		void (*Text)(GB_PAINT *d, const char *text, double x, double y);
		void (*TextExtents)(GB_PAINT *d, const char *text, GB_EXTENTS *ext);
		
		void (*SetBrush)(GB_PAINT *d, GB_BRUSH brush);
		void (*FreeBrush)(GB_PAINT *d, GB_BRUSH brush);
		GB_BRUSH (*ColorBrush)(GB_PAINT *d, GB_COLOR color);
		GB_BRUSH (*ImageBrush)(GB_PAINT *d, GB_IMAGE image, double x, double y, int extend);
		GB_BRUSH (*LinearGradient)(GB_PAINT *d, double x0, double y0, double x1, double y1);
		GB_BRUSH (*RadialGradient)(GB_PAINT *d, double cx0, double cy0, double r0, double cx1, double cy1, double r1);
		void (*SetColorStop)(GB_PAINT *d, GB_BRUSH *brush, int nstop, double *pos, GB_COLOR *color);
		
		void (*Translate)(GB_PAINT *d, double tx, double ty);
		void (*Scale)(GB_PAINT *d, double sx, double sy);
		void (*Rotate)(GB_PAINT *d, double angle);
		void (*Matrix)(GB_PAINT *d, bool set, GB_TRANSFORM *matrix);
	}
	GB_PAINT_DESC;

typedef
	struct {
		int version;
		GB_PAINT *(*GetCurrent)();
		void (*Begin)(void *);
		void (*End)();
		}
	PAINT_INTERFACE;

#define DRAW_NORMALIZE(x, y, w, h, sx, sy, sw, sh, width, height) \
	if (w < 0) w = width; \
	if (h < 0) h = height; \
	if (sw < 0) sw = width; \
	if (sh < 0) sh = height; \
  if (sx >= (width) || sy >= (height) || sw <= 0 || sh <= 0) \
    return; \
  if (sx < 0) x -= sx, sx = 0; \
  if (sy < 0) y -= sy, sy = 0; \
  if (sw > ((width) - sx)) \
    sw = ((width) - sx); \
  if (sh > ((height) - sy)) \
    sh = ((height) - sy);

#endif


 
