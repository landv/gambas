/***************************************************************************

  gb.paint.h

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

enum {
	GB_PAINT_PATH_MOVE,
	GB_PAINT_PATH_LINE
};

struct GB_PAINT_DESC;

typedef
	struct {
		int x, y, w, h;
	}
	GB_RECT;

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
		GB_TRANSFORM transform;
	}
	PAINT_MATRIX;
	
typedef
	struct {
		GB_BASE ob;
		struct GB_PAINT_DESC *desc;        // drawing driver
		GB_BRUSH brush;                    // brush
	}
	PAINT_BRUSH;

typedef
	struct GB_PAINT {
		struct GB_PAINT_DESC *desc;        // drawing driver
		struct GB_PAINT *previous;         // previous drawing context
		void *device;                      // drawing object
		double width;                      // device width in device coordinates
		double height;                     // device height in device coordinates
		int resolutionX;                   // device horizontal resolution in DPI
		int resolutionY;                   // device vertical resolution in DPI
		PAINT_BRUSH *brush;                // current brush
		void *extra;                       // driver-specific state
		unsigned opened : 1;               // if the painting has been opened
		unsigned other : 1;                // if painting are imbricated on that device
		unsigned has_path : 1;             // if there is a current path
		void *tag;                         // needed to support the old Draw class
	}
	GB_PAINT;

typedef
	void (*GB_PAINT_OUTLINE_CB)(int, float, float);

typedef
	struct GB_PAINT_DESC {
		// Size of the GB_PAINT structure extra data
		int size;
		// Begins and terminates the drawing
		int (*Begin)(GB_PAINT *d);
		void (*End)(GB_PAINT *d);
		
		void (*Save)(GB_PAINT *d);
		void (*Restore)(GB_PAINT *d);
		
		void (*Antialias)(GB_PAINT *d, int set, int *antialias);
		
		void (*Font)(GB_PAINT *d, int set, GB_FONT *font);
		
		void (*Background)(GB_PAINT *d, int set, GB_COLOR *color);
		void (*Invert)(GB_PAINT *d, int set, int *invert);

		void (*Clip)(GB_PAINT *d, int preserve);
		void (*ResetClip)(GB_PAINT *d);
		void (*ClipExtents)(GB_PAINT *d, GB_EXTENTS *ext);
		void (*ClipRect)(GB_PAINT *d, int x, int y, int w, int h);
	
		void (*Fill)(GB_PAINT *d, int preserve);
		void (*Stroke)(GB_PAINT *d, int preserve);
		
		void (*PathExtents)(GB_PAINT *d, GB_EXTENTS *ext);
		int (*PathContains)(GB_PAINT *d, float x, float y);
		void (*PathOutline)(GB_PAINT *d, GB_PAINT_OUTLINE_CB cb);
		
		void (*Dash)(GB_PAINT *d, int set, float **dash, int *count);
		void (*DashOffset)(GB_PAINT *d, int set, float *offset);
		
		void (*FillRule)(GB_PAINT *d, int set, int *value);
		void (*FillStyle)(GB_PAINT *d, int set, int *value);
		void (*LineCap)(GB_PAINT *d, int set, int *value);
		void (*LineJoin)(GB_PAINT *d, int set, int *value);
		void (*LineWidth)(GB_PAINT *d, int set, float *value);
		void (*MiterLimit)(GB_PAINT *d, int set, float *value);
		
		void (*Operator)(GB_PAINT *d, int set, int *value);

		void (*NewPath)(GB_PAINT *d);
		void (*ClosePath)(GB_PAINT *d);
		
		void (*Arc)(GB_PAINT *d, float xc, float yc, float radius, float angle, float length, bool pie);
		void (*Ellipse)(GB_PAINT *d, float x, float y, float width, float height, float angle, float length, bool pie);
		void (*Rectangle)(GB_PAINT *d, float x, float y, float width, float height);
		void (*GetCurrentPoint)(GB_PAINT *d, float *x, float *y);
		void (*MoveTo)(GB_PAINT *d, float x, float y);
		void (*LineTo)(GB_PAINT *d, float x, float y);
		void (*CurveTo)(GB_PAINT *d, float x1, float y1, float x2, float y2, float x3, float y3);
	
		void (*Text)(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw);
		void (*TextExtents)(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext);
		void (*TextSize)(GB_PAINT *d, const char *text, int len, float *w, float *h);
		void (*RichText)(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw);
		void (*RichTextExtents)(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext, float width);
		void (*RichTextSize)(GB_PAINT *d, const char *text, int len, float width, float *w, float *h);
		
		void (*Matrix)(GB_PAINT *d, int set, GB_TRANSFORM matrix);
		
		void (*SetBrush)(GB_PAINT *d, GB_BRUSH brush);
		void (*BrushOrigin)(GB_PAINT *d, int set, float *x, float *y);
		
		void (*DrawImage)(GB_PAINT *d, GB_IMAGE image, float x, float y, float w, float h, float opacity, GB_RECT *source);
		void (*DrawPicture)(GB_PAINT *d, GB_PICTURE picture, float x, float y, float w, float h, GB_RECT *source);
		void (*GetPictureInfo)(GB_PAINT *d, GB_PICTURE picture, GB_PICTURE_INFO *info);
		void (*FillRect)(GB_PAINT *d, float x, float y, float width, float height, GB_COLOR color);
		
		struct {
			void (*Free)(GB_BRUSH brush);
			void (*Color)(GB_BRUSH *brush, GB_COLOR color);
			void (*Image)(GB_BRUSH *brush, GB_IMAGE image);
			void (*LinearGradient)(GB_BRUSH *brush, float x0, float y0, float x1, float y1, int nstop, double *positions, GB_COLOR *colors, int extend);
			void (*RadialGradient)(GB_BRUSH *brush, float cx, float cy, float r, float fx, float fy, int nstop, double *positions, GB_COLOR *colors, int extend);
			void (*Matrix)(GB_BRUSH brush, int set, GB_TRANSFORM matrix);
			}
			Brush;
	}
	GB_PAINT_DESC;

typedef
	struct GB_PAINT_MATRIX_DESC {
		void (*Create)(GB_TRANSFORM *matrix);
		void (*Copy)(GB_TRANSFORM *matrix, GB_TRANSFORM copy);
		void (*Delete)(GB_TRANSFORM *matrix);
		void (*Init)(GB_TRANSFORM matrix, float xx, float yx, float xy, float yy, float x0, float y0);
		void (*Translate)(GB_TRANSFORM matrix, float tx, float ty);
		void (*Scale)(GB_TRANSFORM matrix, float sx, float sy);
		void (*Rotate)(GB_TRANSFORM matrix, float angle);
		int (*Invert)(GB_TRANSFORM matrix);
		void (*Multiply)(GB_TRANSFORM matrix, GB_TRANSFORM matrix2);
		void (*Map)(GB_TRANSFORM matrix, double *x, double *y);
		}
	GB_PAINT_MATRIX_DESC;

#endif


 
