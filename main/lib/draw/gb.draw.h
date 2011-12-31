/***************************************************************************

  gb.draw.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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
	struct _GB_MATRIX {
		double m11, m12, m21, m22;
		double dx, dy;
		unsigned identity : 1;
		unsigned rotation : 1;
		struct _GB_MATRIX *next;
		}
	GB_MATRIX;

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

struct GB_DRAW_DESC;

typedef
	struct GB_DRAW {
		struct GB_DRAW_DESC *desc;         // drawing driver
		struct GB_DRAW *previous;          // previous drawing context
		void *device;                      // drawing object
		int width;                         // device width in device coordinates
		int height;                        // device height in device coordinates
		int resolution;                    // device resolution in DPI
		unsigned xform : 1;                // if the matrix must be used
		unsigned opened : 1;               // if the drawing has been opened
		unsigned _reserved : 30;
		GB_MATRIX matrix;                  // transformation matrix (do not manage rotations)
		GB_MATRIX *save;                   // transformation matrix stack
		void *paint;                       // if the device was painted before calling Draw.Begin
		void *extra;                       // driver-specific state
	}
	PACKED
	GB_DRAW;

typedef
	struct GB_DRAW_DESC {
		// Size of the GB_DRAW structure extra data
		int size;
		// Begins and terminates the drawing
		int (*Begin)(GB_DRAW *d);
		void (*End)(GB_DRAW *d);
		// Save / restore state
		void (*Save)(GB_DRAW *d);
		void (*Restore)(GB_DRAW *d);
		// Default colors
		int (*GetBackground)(GB_DRAW *d);
		void (*SetBackground)(GB_DRAW *d, int color);
		int (*GetForeground)(GB_DRAW *d);
		void (*SetForeground)(GB_DRAW *d, int color);
		// Default font
		GB_FONT (*GetFont)(GB_DRAW *d);
		void (*SetFont)(GB_DRAW *d, GB_FONT font);
		// Drawing mode
		int (*IsInverted)(GB_DRAW *d);
		void (*SetInverted)(GB_DRAW *d, int inverted);
		int (*IsTransparent)(GB_DRAW *d);
		void (*SetTransparent)(GB_DRAW *d, int transparent);
		// Picture properties
		void (*GetPictureInfo)(GB_DRAW *d, GB_PICTURE picture, GB_PICTURE_INFO *info);
		// Line properties
		struct {
			int (*GetWidth)(GB_DRAW *d);
			void (*SetWidth)(GB_DRAW *d, int width);
			int (*GetStyle)(GB_DRAW *d);
			void (*SetStyle)(GB_DRAW *d, int style);
			}
			Line;
		// Fill properties
		struct {
			int (*GetColor)(GB_DRAW *d);
			void (*SetColor)(GB_DRAW *d, int color);
			int (*GetStyle)(GB_DRAW *d);
			void (*SetStyle)(GB_DRAW *d, int style);
			void (*GetOrigin)(GB_DRAW *d, int *x, int *y);
			void (*SetOrigin)(GB_DRAW *d, int x, int y);
			}
			Fill;
		// Drawing methods
		struct {
			void (*Rect)(GB_DRAW *d, int x, int y, int w, int h);
			//void (*RoundRect)(GB_DRAW *d, int x, int y, int w, int h, double round);
			void (*Ellipse)(GB_DRAW *d, int x, int y, int w, int h, double start, double end);
			void (*Arc)(GB_DRAW *d, int x, int y, int w, int h, double start, double end);
			void (*Line)(GB_DRAW *d, int x1, int y1, int x2, int y2);
			void (*Point)(GB_DRAW *d, int x, int y);
			void (*Picture)(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h, int sx, int sy, int sw, int sh); 
			void (*Image)(GB_DRAW *d, GB_IMAGE image, int x, int y, int w, int h, int sx, int sy, int sw, int sh); 
			void (*TiledPicture)(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h);
			void (*Text)(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align);
			void (*TextSize)(GB_DRAW *d, char *text, int len, int *w, int *h);
			void (*Polyline)(GB_DRAW *d, int count, int *points);
			void (*Polygon)(GB_DRAW *d, int count, int *points);
			void (*RichText)(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align);
			void (*RichTextSize)(GB_DRAW *d, char *text, int len, int sw, int *w, int *h);
			}
			Draw;
		// Clipping methods
		struct {
			void (*Get)(GB_DRAW *d, int *x, int *y, int *w, int *h);
			void (*Set)(GB_DRAW *d, int x, int y, int w, int h);
			int (*IsEnabled)(GB_DRAW *d);
			void (*SetEnabled)(GB_DRAW *d, int enable);
			}
			Clip;
		// Style methods
		struct {
			void (*Arrow)(GB_DRAW *d, int x, int y, int w, int h, int type, int state);
			void (*Check)(GB_DRAW *d, int x, int y, int w, int h, int value, int state);
			void (*Option)(GB_DRAW *d, int x, int y, int w, int h, int value, int state);
			void (*Separator)(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state);
			void (*Button)(GB_DRAW *d, int x, int y, int w, int h, int value, int state, int flat);
			void (*Panel)(GB_DRAW *d, int x, int y, int w, int h, int border, int state);
			void (*Handle)(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state);
			void (*Box)(GB_DRAW *d, int x, int y, int w, int h, int state);
			}
			Style;
	}
	GB_DRAW_DESC;

#define DRAW_INTERFACE_VERSION 1

typedef
	struct {
		int version;
		GB_DRAW *(*GetCurrent)();
		void (*Begin)(void *);
		void (*End)();
		struct {
			void *(*GetCurrent)();
			void (*Begin)(void *);
			void (*End)();
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


 
