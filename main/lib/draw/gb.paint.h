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
	GB_PAINT_OPERATOR_DEST_OUT,
	GB_PAINT_OPERATOR_DEST_ATOP,
	GB_PAINT_OPERATOR_XOR,
	GB_PAINT_OPERATOR_ADD,
	GB_PAINT_OPERATOR_SATURATE
};

struct GB_PAINT_DESC;

typedef
	struct {
		float x1, y1, x2, y2;
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
		struct GB_PAINT_DESC *desc;        // drawing driver
		GB_TRANSFORM matrix;
	}
	PAINT_MATRIX;
	
typedef
	struct {
		GB_BASE ob;
		struct GB_PAINT_DESC *desc;        // drawing driver
		GB_BRUSH brush;
	}
	PAINT_BRUSH;

typedef
	struct GB_PAINT {
		struct GB_PAINT_DESC *desc;        // drawing driver
		struct GB_PAINT *previous;         // previous drawing context
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
		bool (*PathContains)(GB_PAINT *d, float x, float y);
		
		void (*Dash)(GB_PAINT *d, bool set, float **dash, int *count);
		void (*DashOffset)(GB_PAINT *d, bool set, float *offset);
		
		void (*FillRule)(GB_PAINT *d, bool set, int *value);
		void (*LineCap)(GB_PAINT *d, bool set, int *value);
		void (*LineJoin)(GB_PAINT *d, bool set, int *value);
		void (*LineWidth)(GB_PAINT *d, bool set, float *value);
		void (*MiterLimit)(GB_PAINT *d, bool set, float *value);
		
		void (*Operator)(GB_PAINT *d, bool set, int *value);

		void (*NewPath)(GB_PAINT *d);
		void (*ClosePath)(GB_PAINT *d);
		
		void (*Arc)(GB_PAINT *d, float xc, float yc, float radius, float a1, float a2);
		void (*Rectangle)(GB_PAINT *d, float x, float y, float width, float height);
		void (*GetCurrentPoint)(GB_PAINT *d, float *x, float *y);
		void (*MoveTo)(GB_PAINT *d, float x, float y);
		void (*LineTo)(GB_PAINT *d, float x, float y);
		void (*CurveTo)(GB_PAINT *d, float x1, float y1, float x2, float y2, float x3, float y3);
	
		void (*Text)(GB_PAINT *d, const char *text, int len);
		void (*TextExtents)(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext);
		
		void (*Translate)(GB_PAINT *d, float tx, float ty);
		void (*Scale)(GB_PAINT *d, float sx, float sy);
		void (*Rotate)(GB_PAINT *d, float angle);
		void (*Matrix)(GB_PAINT *d, bool set, GB_TRANSFORM *matrix);
		
		void (*SetBrush)(GB_PAINT *d, GB_BRUSH brush);
		
		struct {
			void (*Free)(GB_BRUSH brush);
			void (*Color)(GB_BRUSH *brush, GB_COLOR color);
			void (*Image)(GB_BRUSH *brush, GB_IMAGE image, float x, float y, int extend);
			void (*LinearGradient)(GB_BRUSH *brush, float x0, float y0, float x1, float y1);
			void (*RadialGradient)(GB_BRUSH *brush, float cx0, float cy0, float r0, float cx1, float cy1, float r1);
			void (*SetColorStops)(GB_BRUSH brush, int nstop, double *pos, GB_COLOR *color);
			void (*Matrix)(GB_BRUSH brush, bool set, GB_TRANSFORM *matrix);
			}
			Brush;
		
		struct {
			void (*Init)(GB_TRANSFORM matrix, float xx, float yx, float xy, float yy, float x0, float y0);
			void (*Translate)(GB_TRANSFORM matrix, float tx, float ty);
			void (*Scale)(GB_TRANSFORM matrix, float sx, float sy);
			void (*Rotate)(GB_TRANSFORM matrix, float angle);
			bool (*Invert)(GB_TRANSFORM matrix);
			void (*Multiply)(GB_TRANSFORM matrix, GB_TRANSFORM matrix2);
			}
			Transform;
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

#endif


 
