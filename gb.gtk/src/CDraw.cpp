/***************************************************************************

  CDraw.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#define __CDRAW_CPP

#include "CWindow.h"
#include "CDrawingArea.h"
#include "CPicture.h"
#include "CImage.h"
#include "CFont.h"
#include "CDraw.h"

typedef
	struct {
		gDraw *dr;
		CFONT *font;
		}
	GB_DRAW_EXTRA;

#define EXTRA(d) ((GB_DRAW_EXTRA *)(&(d->extra)))
#define DR(d) (EXTRA(d)->dr)
 
DRAW_INTERFACE DRAW EXPORT;

void DRAW_init()
{
	GB.GetInterface("gb.draw", DRAW_INTERFACE_VERSION, &DRAW);
}

static int begin(GB_DRAW *d)
{
	gDraw *dr;
	
	EXTRA(d)->dr = dr = new gDraw();
	EXTRA(d)->font = NULL;
	
	if (GB.Is(d->device, CLASS_Window))
		dr->connect(((CWINDOW *)d->device)->ob.widget);
	else if (GB.Is(d->device, CLASS_DrawingArea))
		dr->connect(((CDRAWINGAREA *)d->device)->ob.widget);
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
	GB.Unref(POINTER(&(EXTRA(d)->font)));
	EXTRA(d)->font = NULL;
	delete dr;
}

static void save(GB_DRAW *d)
{
	DR(d)->save();
}

static void restore(GB_DRAW *d)
{
	DR(d)->restore();
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
	if (!EXTRA(d)->font)
	{
		EXTRA(d)->font = CFONT_create(DR(d)->font()->copy(), apply_font);
		GB.Ref(EXTRA(d)->font);
	}
		
	return (GB_FONT)EXTRA(d)->font;
}

static void set_font(GB_DRAW *d, GB_FONT font)
{
	if (font)
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

static void get_picture_info(GB_DRAW *d, GB_PICTURE picture, GB_PICTURE_INFO *info)
{
	gPicture *pic = ((CPICTURE *)picture)->picture;
	
	info->width = pic->width();
	info->height = pic->height();
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
	gPicture *pic = CIMAGE_get((CIMAGE *)image);
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

static void style_arrow(GB_DRAW *d, int x, int y, int w, int h, int type, int state)
{
	GtkArrowType arrow;
	
	switch (type)
	{
		case ALIGN_NORMAL: arrow = GB.System.IsRightToLeft() ? GTK_ARROW_LEFT : GTK_ARROW_RIGHT; break;
		case ALIGN_LEFT: arrow = GTK_ARROW_LEFT; break;
		case ALIGN_RIGHT: arrow = GTK_ARROW_RIGHT; break;
		case ALIGN_TOP: arrow = GTK_ARROW_UP; break;
		case ALIGN_BOTTOM: arrow = GTK_ARROW_DOWN; break;
		default:
			return;
	}
	
	gtk_paint_arrow(DR(d)->style(), DR(d)->drawable(), 
		state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL, 
		GTK_SHADOW_NONE, NULL, NULL, NULL, 
		arrow, TRUE, x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_arrow(DR(d)->style(), DR(d)->mask(), 
			state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL, 
			GTK_SHADOW_NONE, NULL, NULL, NULL, 
			arrow, TRUE, x, y, w, h);
}

static void style_check(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	GtkShadowType shadow;
	GtkStateType st = state ? GTK_STATE_INSENSITIVE : (value ? GTK_STATE_ACTIVE : GTK_STATE_NORMAL);
	
	switch (value)
	{
		case -1: shadow = GTK_SHADOW_IN; break;
		case 1: shadow = GTK_SHADOW_ETCHED_IN; break;
		default: shadow = GTK_SHADOW_OUT; break;
	}

	gtk_paint_check(DR(d)->style(), DR(d)->drawable(),
		st, 
		shadow, NULL, NULL, "checkbutton", 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_check(DR(d)->style(), DR(d)->mask(),
			st, 
			shadow, NULL, NULL, "checkbutton", 
			x, y, w, h);
}

static void style_option(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	GtkShadowType shadow;
	GtkStateType st = state ? GTK_STATE_INSENSITIVE : (value ? GTK_STATE_ACTIVE : GTK_STATE_NORMAL);
	
	shadow = value ? GTK_SHADOW_IN : GTK_SHADOW_OUT;

	gtk_paint_option(DR(d)->style(), DR(d)->drawable(),
		st, shadow, NULL, NULL, "radiobutton", 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_option(DR(d)->style(), DR(d)->mask(),
			st, shadow, NULL, NULL, "radiobutton", 
			x, y, w, h);
}

static void style_separator(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	GtkStateType st = state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL;
	
	if (vertical)
	{
		gtk_paint_vline(DR(d)->style(), DR(d)->drawable(),
			st, 
			NULL, NULL, NULL, 
			y, y + h - 1, x + (w / 2));
		if (DR(d)->mask())
			gtk_paint_vline(DR(d)->style(), DR(d)->mask(),
				st, 
				NULL, NULL, NULL, 
				y, y + h - 1, x + (w / 2));
	}
	else
	{
		gtk_paint_hline(DR(d)->style(), DR(d)->drawable(),
			st, 
			NULL, NULL, NULL, 
			x, x + w - 1, y + (h / 2));
		if (DR(d)->mask())
			gtk_paint_hline(DR(d)->style(), DR(d)->mask(),
				st, 
				NULL, NULL, NULL, 
				x, x + w - 1, y + (h / 2));
	}
}

static void style_focus(GB_DRAW *d, int x, int y, int w, int h)
{
	gtk_paint_focus(DR(d)->style(), DR(d)->drawable(),
		GTK_STATE_NORMAL, 
		NULL, NULL, NULL, 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_focus(DR(d)->style(), DR(d)->mask(),
			GTK_STATE_NORMAL, 
			NULL, NULL, NULL, 
			x, y, w, h);
}
			
static void style_button(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	GtkStateType st = state ? GTK_STATE_INSENSITIVE : (value ? GTK_STATE_ACTIVE : GTK_STATE_NORMAL);
	
	gtk_paint_box(DR(d)->style(), DR(d)->drawable(),
		st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
		NULL, NULL, "button",
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_box(DR(d)->style(), DR(d)->mask(),
			st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
			NULL, NULL, "button",
			x, y, w, h);
}
			
static void style_panel(GB_DRAW *d, int x, int y, int w, int h, int border, int state)
{
	GtkShadowType shadow;

	switch (border)
	{
		case BORDER_SUNKEN: shadow = GTK_SHADOW_IN; break;
		case BORDER_RAISED: shadow = GTK_SHADOW_OUT; break;
		case BORDER_ETCHED: shadow = GTK_SHADOW_ETCHED_IN; break;
		default: shadow = GTK_SHADOW_NONE;
	}
	
	gtk_paint_shadow(DR(d)->style(), DR(d)->drawable(), 
		state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL, 
		shadow, NULL, NULL, NULL, 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_shadow(DR(d)->style(), DR(d)->mask(), 
			state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL, 
			shadow, NULL, NULL, NULL, 
			x, y, w, h);
	
	if (border == BORDER_PLAIN)
	{
		gdk_draw_rectangle(DR(d)->drawable(), DR(d)->style()->fg_gc[state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL], FALSE, x, y, w - 1, h - 1); 
		if (DR(d)->mask())
			gdk_draw_rectangle(DR(d)->mask(), DR(d)->style()->fg_gc[state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL], FALSE, x, y, w - 1, h - 1); 
	}
}
			
static void style_handle(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	gtk_paint_handle(DR(d)->style(), DR(d)->drawable(),
		state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL, 
		GTK_SHADOW_NONE, NULL, NULL, NULL,
		x, y, w, h,
		(!vertical) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
	if (DR(d)->mask())
		gtk_paint_handle(DR(d)->style(), DR(d)->mask(),
			state ? GTK_STATE_INSENSITIVE : GTK_STATE_NORMAL, 
			GTK_SHADOW_NONE, NULL, NULL, NULL,
			x, y, w, h,
			(!vertical) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
}
			
			
GB_DRAW_DESC DRAW_Interface = {
	sizeof(GB_DRAW_EXTRA),
	begin,
	end,
	save,
	restore,
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
	get_picture_info,
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
	},
	{
		style_arrow,
		style_check,
		style_option,
		style_separator,
		style_focus,
		style_button,
		style_panel,
		style_handle
	}
};

void DRAW_begin(void *device)
{
	DRAW.Begin(device);
}

void DRAW_end()
{
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




