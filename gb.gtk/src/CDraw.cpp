/***************************************************************************

  CDraw.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  GTK+ component

  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CDRAW_CPP

#include "CWindow.h"
#include "CDrawingArea.h"
#include "CPicture.h"
#include "CFont.h"
#include "CDraw.h"

typedef
	struct {
		gDraw *dr;
		}
	GB_DRAW_EXTRA;

#define EXTRA(d) ((GB_DRAW_EXTRA *)(&(d->extra)))
#define DR(d) (EXTRA(d)->dr)
 
DRAW_INTERFACE DRAW EXPORT;

static bool _init = FALSE;
static void *CLASS_Window;
static void *CLASS_Picture;
static void *CLASS_DrawingArea;

static void init()
{
	if (_init)
		return;
		
	GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);
	CLASS_Window = GB.FindClass("Window");
	CLASS_Picture = GB.FindClass("Picture");
	CLASS_DrawingArea = GB.FindClass("DrawingArea");
	_init = TRUE;
}

static int begin(GB_DRAW *d)
{
	gDraw *dr;
	
	init();
	EXTRA(d)->dr = dr = new gDraw();
	
	if (GB.Is(d->device, CLASS_Window))
		dr->connect(((CWINDOW *)d->device)->widget);
	else if (GB.Is(d->device, CLASS_DrawingArea))
		dr->connect(((CDRAWINGAREA *)d->device)->widget);
	else if (GB.Is(d->device, CLASS_Picture))
	{
		gPicture *pic = ((CPICTURE*)d->device)->picture;
		if (pic->isVoid())
		{
			GB.Error("Bad picture");
			return TRUE;
		}
		dr->connect(pic);
	}
		
	d->width = dr->width();
	d->height = dr->height();
	d->resolution = dr->resolution();
	return FALSE;
}

static void end(GB_DRAW *d)
{
	gDraw *dr = DR(d);
	delete dr;
}

static int get_background(GB_DRAW *d)
{
	return DR(d)->background();
}

static void set_background(GB_DRAW *d, int col)
{
	DR(d)->setBackground(col);
}

static int get_foreground(GB_DRAW *d)
{
	return DR(d)->foreground();
}

static void set_foreground(GB_DRAW *d, int col)
{
	DR(d)->setForeground(col);
}

static void apply_font(gFont *font, void *object = 0)
{
	GB_DRAW *d = DRAW.GetCurrent();
	if (!d)
		return;
		
  DR(d)->setFont(font);
}

static GB_FONT get_font(GB_DRAW *d)
{
	return CFONT_create(DR(d)->font(), apply_font);
}

static void set_font(GB_DRAW *d, GB_FONT font)
{
	apply_font(((CFONT*)font)->font);
}

static int is_inverted(GB_DRAW *d)
{
	return DR(d)->invert();
}

static void set_inverted(GB_DRAW *d, int inverted)
{
	DR(d)->setInvert(inverted);
}

static int is_transparent(GB_DRAW *d)
{
	return DR(d)->isTransparent();
}

static void set_transparent(GB_DRAW *d, int transparent)
{
	DR(d)->setTransparent(transparent);
}

static int get_line_width(GB_DRAW *d)
{
	return DR(d)->lineWidth();
}

static void set_line_width(GB_DRAW *d, int width)
{
	DR(d)->setLineWidth(width);
}

static int get_line_style(GB_DRAW *d)
{
	return DR(d)->lineStyle();
}

static void set_line_style(GB_DRAW *d, int style)
{
	DR(d)->setLineStyle(style);
}

static int get_fill_color(GB_DRAW *d)
{
	return DR(d)->fillColor();
}

static void set_fill_color(GB_DRAW *d, int color)
{
	DR(d)->setFillColor(color);
}

static int get_fill_style(GB_DRAW *d)
{
	return DR(d)->fillStyle();
}

static void set_fill_style(GB_DRAW *d, int style)
{
	DR(d)->setFillStyle(style);
}

static void get_fill_origin(GB_DRAW *d, int *x, int *y)
{
	if (x) *x = DR(d)->fillX();
	if (y) *y = DR(d)->fillY();
}

static void set_fill_origin(GB_DRAW *d, int x, int y)
{
	DR(d)->setFillX(x);
	DR(d)->setFillY(y);
}

static void draw_rect(GB_DRAW *d, int x, int y, int w, int h)
{
	DR(d)->rect(x, y, w, h);
}

static void draw_ellipse(GB_DRAW *d, int x, int y, int w, int h, double start, double end)
{
	DR(d)->ellipse(x, y, w, h, start, end);
}

static void draw_line(GB_DRAW *d, int x1, int y1, int x2, int y2)
{
	DR(d)->line(x1, y1, x2, y2);
}

static void draw_point(GB_DRAW *d, int x, int y)
{
	DR(d)->point(x, y);
}

static void draw_picture(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	gPicture *pic = ((CPICTURE *)picture)->picture;
	DR(d)->picture(pic, x, y, w, h, sx, sy, sw, sh);
}

static void draw_image(GB_DRAW *d, GB_IMAGE image, int x, int y, int w, int h, int sx, int sy, int sw, int sh)
{
	gPicture *pic = ((CIMAGE *)image)->picture;
	DR(d)->picture(pic, x, y, w, h, sx, sy, sw, sh);
}

static void draw_tiled_picture(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h)
{
	gPicture *pic = ((CPICTURE *)picture)->picture;
	DR(d)->tiledPicture(pic, x, y, w, h);
}

static void draw_text(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align)
{
	if (align == GB_DRAW_ALIGN_DEFAULT)
		align = ALIGN_TOP_NORMAL;
		
	DR(d)->text(text, len, x, y, w, h, align);
}

static void text_size(GB_DRAW *d, char *text, int len, int *w, int *h)
{
	if (w) *w = DR(d)->textWidth(text, len);
	if (h) *h = DR(d)->textHeight(text, len);
}

static void draw_rich_text(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align)
{
	if (align == GB_DRAW_ALIGN_DEFAULT)
		align = ALIGN_TOP_NORMAL;
		
	DR(d)->richText(text, len, x, y, w, h, align);
}

static void rich_text_size(GB_DRAW *d, char *text, int len, int sw, int *w, int *h)
{
	DR(d)->richTextSize(text, len, sw, w, h);
}

static void draw_polyline(GB_DRAW *d, int count, int *points)
{
	DR(d)->polyline(points, count);
}

static void draw_polygon(GB_DRAW *d, int count, int *points)
{
	DR(d)->polygon(points, count);
}

static void get_clipping(GB_DRAW *d, int *x, int *y, int *w, int *h)
{
	if (x) *x = DR(d)->clipX();
	if (y) *y = DR(d)->clipY();
	if (w) *w = DR(d)->clipWidth();
	if (h) *h = DR(d)->clipHeight();
}

static void set_clipping(GB_DRAW *d, int x, int y, int w, int h)
{
	DR(d)->setClip(x, y, w, h);
}

int is_clipping_enabled(GB_DRAW *d)
{
	return DR(d)->clipEnabled();
}

static void set_clipping_enabled(GB_DRAW *d, int enable)
{
	DR(d)->setClipEnabled(enable);
}

#if 0
		// Drawing methods
		struct {
			void (*Rect)(GB_DRAW *d, int x, int y, int w, int h);
			void (*Ellipse)(GB_DRAW *d, int x, int y, int w, int h, double start, double end);
			void (*Line)(GB_DRAW *d, int x1, int y1, int x2, int y2);
			void (*Point)(GB_DRAW *d, int x, int y);
			void (*Picture)(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h, int sx, int sy, int sw, int sh); 
			void (*Image)(GB_DRAW *d, GB_IMAGE image, int x, int y, int w, int h, int sx, int sy, int sw, int sh); 
			void (*ZoomedImage)(GB_DRAW *d, GB_IMAGE image, int zoom, int x, int y, int sx, int sy, int sw, int sh); 
			void (*TiledPicture)(GB_DRAW *d, GB_PICTURE picture, int x, int y, int w, int h);
			void (*Text)(GB_DRAW *d, char *text, int len, int x, int y, int w, int h, int align);
			int (*TextWidth)(GB_DRAW *d, char *text, int len);
			int (*TextHeight)(GB_DRAW *d, char *text, int len);
			void (*Polyline)(GB_DRAW *d, int count, int *points);
			void (*Polygon)(GB_DRAW *d, int count, int *points);
			}
			Draw;
		// Clipping
		struct {
			void (*Get)(GB_DRAW *d, int *x, int *y, int *w, int *h);
			void (*Set)(GB_DRAW *d, int x, int y, int w, int h);
			void (*SetEnabled)(GB_DRAW *d, int enable);
			int (*IsEnabled)(GB_DRAW *d);
			}
			Clip;
#endif


GB_DRAW_DESC DRAW_Interface = {
	sizeof(GB_DRAW_EXTRA),
	begin,
	end,
	get_background,
	set_background,
	get_foreground,
	set_foreground,
	get_font,
	set_font,
	is_inverted,
	set_inverted,
	is_transparent,
	set_transparent,
	{
		get_line_width,
		set_line_width,
		get_line_style,
		set_line_style
	},
	{
		get_fill_color,
		set_fill_color,
		get_fill_style,
		set_fill_style,
		get_fill_origin,
		set_fill_origin
	},
	{
		draw_rect,
		draw_ellipse,
		draw_line,
		draw_point,
		draw_picture,
		draw_image,
		draw_tiled_picture,
		draw_text,
		text_size,
		draw_polyline,
		draw_polygon,
		draw_rich_text,
		rich_text_size
	},
	{
		get_clipping,
		set_clipping,
		is_clipping_enabled,
		set_clipping_enabled
	}
};

void DRAW_begin(void *device)
{
	init();
	DRAW.Begin(device);
}

void DRAW_end()
{
	init();
	DRAW.End();
}

gDraw *DRAW_get_current()
{
	GB_DRAW *current = DRAW.GetCurrent(); 
	return current ? DR(current) : NULL;	
}

void *DRAW_get_drawable(void *dr)
{
	if (!dr) return NULL;
	return (void*)((gDraw*)dr)->drawable();
}

void *DRAW_get_style(void *dr)
{
	if (!dr) return NULL;
	return (void*)((gDraw*)dr)->style();
}

int DRAW_get_state(void *dr)
{
	if (!dr) return GTK_STATE_NORMAL;
	return ((gDraw*)dr)->state();
}

int DRAW_get_shadow(void *dr)
{
	if (!dr) return GTK_SHADOW_NONE;
	return ((gDraw*)dr)->shadow();
}

void DRAW_set_state(void *dr,int vl)
{
	if (!dr) return;
	((gDraw*)dr)->setState(vl);
}

void DRAW_set_shadow(void *dr,int vl)
{
	if (!dr) return;
	((gDraw*)dr)->setShadow(vl);
}




