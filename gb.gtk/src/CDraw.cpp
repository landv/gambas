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
#include "gdesktop.h"

typedef
	struct {
		gDraw *dr;
		CFONT *font;
		}
	GB_DRAW_EXTRA;

#define EXTRA(d) ((GB_DRAW_EXTRA *)(d->extra))
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

static GtkStateType get_state(int state)
{
	if (state & GB_DRAW_STATE_DISABLED)
		return GTK_STATE_INSENSITIVE;
	if (state & GB_DRAW_STATE_ACTIVE)
		return GTK_STATE_ACTIVE;
	if (state & GB_DRAW_STATE_HOVER)
		return GTK_STATE_PRELIGHT;
	
	return GTK_STATE_NORMAL;
}

static GdkRectangle *get_area(GB_DRAW *d)
{
	static GdkRectangle area;
	
	if (!DR(d)->clipEnabled())
		return NULL;
	
	get_clipping(d, &area.x, &area.y, &area.width, &area.height);
	return &area;
}

static void paint_focus(GB_DRAW *d, GtkStyle *style, int x, int y, int w, int h, GtkStateType state, const char *kind)
{
	gtk_paint_focus(style, DR(d)->drawable(),
		state, 
		get_area(d), DR(d)->widget(), kind, 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_focus(style, DR(d)->mask(),
			state, 
			get_area(d), DR(d)->widget(), kind, 
			x, y, w, h);
}

static void style_arrow(GB_DRAW *d, int x, int y, int w, int h, int type, int state)
{
	GtkArrowType arrow;
	GtkStyle *style = DR(d)->style();
	
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
	
	gtk_paint_arrow(style, DR(d)->drawable(), get_state(state), 
		GTK_SHADOW_NONE, get_area(d), NULL, NULL, 
		arrow, TRUE, x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_arrow(style, DR(d)->mask(), get_state(state), 
			GTK_SHADOW_NONE, get_area(d), NULL, NULL, 
			arrow, TRUE, x, y, w, h);
}

static void style_check(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	GtkShadowType shadow;
	GtkStateType st = get_state(state | (value ? GB_DRAW_STATE_ACTIVE : 0));
	GtkStyle *style = DR(d)->style("GtkCheckButton", GTK_TYPE_CHECK_BUTTON);
	
	switch (value)
	{
		case -1: shadow = GTK_SHADOW_IN; break;
		case 1: shadow = GTK_SHADOW_ETCHED_IN; break;
		default: shadow = GTK_SHADOW_OUT; break;
	}

	gtk_paint_check(style, DR(d)->drawable(),
		st, shadow, get_area(d), NULL, "checkbutton", 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_check(style, DR(d)->mask(),
			st, shadow, get_area(d), NULL, "checkbutton", 
			x, y, w, h);

	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(d, style, x, y, w, h, st, "checkbutton");
}

static void style_option(GB_DRAW *d, int x, int y, int w, int h, int value, int state)
{
	GtkShadowType shadow;
	GtkStateType st = get_state(state | (value ? GB_DRAW_STATE_ACTIVE : 0));
	GtkStyle *style = DR(d)->style("GtkRadioButton", GTK_TYPE_RADIO_BUTTON);
	
	shadow = value ? GTK_SHADOW_IN : GTK_SHADOW_OUT;

	gtk_paint_option(style, DR(d)->drawable(),
		st, shadow, get_area(d), NULL, "radiobutton", 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_option(style, DR(d)->mask(),
			st, shadow, get_area(d), NULL, "radiobutton", 
			x, y, w, h);
			
	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(d, style, x, y, w, h, st, "radiobutton");
}

static void style_separator(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	GtkStateType st = get_state(state);
	GtkStyle *style = DR(d)->style();
	
	if (vertical)
	{
		gtk_paint_vline(style, DR(d)->drawable(),
			st, get_area(d), NULL, NULL, 
			y, y + h - 1, x + (w / 2));
		if (DR(d)->mask())
			gtk_paint_vline(style, DR(d)->mask(),
				st, get_area(d), NULL, NULL, 
				y, y + h - 1, x + (w / 2));
	}
	else
	{
		gtk_paint_hline(style, DR(d)->drawable(),
			st, get_area(d), NULL, NULL, 
			x, x + w - 1, y + (h / 2));
		if (DR(d)->mask())
			gtk_paint_hline(style, DR(d)->mask(),
				st, get_area(d), NULL, NULL, 
				x, x + w - 1, y + (h / 2));
	}
}

static void style_button(GB_DRAW *d, int x, int y, int w, int h, int value, int state, int flat)
{
	GtkStateType st = get_state(state | (value ? GB_DRAW_STATE_ACTIVE : 0));
	GtkStyle *style = DR(d)->style("GtkButton", GTK_TYPE_BUTTON);
	int xf, yf, wf, hf;
	GtkBorder *default_border, *default_outside_border, *inner_border;
	int focus_width, focus_pad, df;
	gboolean interior_focus;
	
	gtk_style_get(style, GTK_TYPE_BUTTON,
		"default-border", &default_border,
		"default-outside-border", &default_outside_border,
		"inner-border", &inner_border,
		"focus-line-width", &focus_width,
		"focus-padding", &focus_pad,
		"interior-focus", &interior_focus,
		(char *)NULL); 

	/*if (default_outside_border)
	{
		x += default_outside_border->left;
		y += default_outside_border->top;
		w -= default_outside_border->left + default_outside_border->right;
		h -= default_outside_border->top + default_outside_border->bottom;
	}*/
	
	if (default_border)
	{
		x += default_border->left;
		y += default_border->top;
		w -= default_border->left + default_border->right;
		h -= default_border->top + default_border->bottom;
	}

	if (inner_border) gtk_border_free(inner_border);
	if (default_outside_border) gtk_border_free(default_outside_border);
	if (default_border) gtk_border_free(default_border);
	
	xf = x;
	yf = y;
	wf = w;
	hf = h;
		
	if (interior_focus)
	{
		df = focus_pad + style->xthickness;
		xf += df;
		wf -= df * 2;
		df = focus_pad + style->ythickness;
		yf += df;
		hf -= df * 2;
	}
	else if (state & GB_DRAW_STATE_FOCUS)
	{
		df = focus_pad + focus_width;
		
		x += df;
		w -= df * 2;
		y += df;
		h -= df * 2;
	}
	
	if (flat && (state & GB_DRAW_STATE_HOVER) == 0)
	{
		/*gtk_paint_flat_box(style, DR(d)->drawable(),
			st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
			get_area(d), DR(d)->widget(), "button",
			x, y, w, h);
		if (DR(d)->mask())
			gtk_paint_flat_box(style, DR(d)->mask(),
				st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
				get_area(d), DR(d)->widget(), "button",
				x, y, w, h);*/
	}
	else
	{
		gtk_paint_box(style, DR(d)->drawable(),
			st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
			get_area(d), DR(d)->widget(), "button",
			x, y, w, h);
		if (DR(d)->mask())
			gtk_paint_box(style, DR(d)->mask(),
				st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
				get_area(d), DR(d)->widget(), "button",
				x, y, w, h);
	}

	if (state & GB_DRAW_STATE_FOCUS)
	{
		paint_focus(d, style, xf, yf, wf, hf, st, "button");
	}
}
			
static void style_panel(GB_DRAW *d, int x, int y, int w, int h, int border, int state)
{
	GtkShadowType shadow;
	GtkStateType st = get_state(state);
	GtkStyle *style = DR(d)->style();

	switch (border)
	{
		case BORDER_SUNKEN: shadow = GTK_SHADOW_IN; break;
		case BORDER_RAISED: shadow = GTK_SHADOW_OUT; break;
		case BORDER_ETCHED: shadow = GTK_SHADOW_ETCHED_IN; break;
		default: shadow = GTK_SHADOW_NONE;
	}
	
	gtk_paint_shadow(style, DR(d)->drawable(), 
		st, shadow, get_area(d), NULL, NULL, 
		x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_shadow(style, DR(d)->mask(), 
			st, shadow, get_area(d), NULL, NULL, 
			x, y, w, h);
	
	if (border == BORDER_PLAIN)
	{
		GdkGC *gc;
		GdkGCValues values;
		uint col;
		
		col = IMAGE.MergeColor(gDesktop::bgColor(), gDesktop::fgColor(), 0.5);
		col = IMAGE.LighterColor(col);

		fill_gdk_color(&values.foreground, col, gdk_drawable_get_colormap(DR(d)->drawable()));
		gc = gtk_gc_get(gdk_drawable_get_depth(DR(d)->drawable()), gdk_drawable_get_colormap(DR(d)->drawable()), &values, GDK_GC_FOREGROUND);
		gdk_draw_rectangle(DR(d)->drawable(), gc, FALSE, x, y, w - 1, h - 1); 
		gtk_gc_release(gc);
		
		if (DR(d)->mask())
		{
			fill_gdk_color(&values.foreground, col, gdk_drawable_get_colormap(DR(d)->mask()));
			gc = gtk_gc_get(gdk_drawable_get_depth(DR(d)->mask()), gdk_drawable_get_colormap(DR(d)->mask()), &values, GDK_GC_FOREGROUND);
			gdk_draw_rectangle(DR(d)->mask(), gc, FALSE, x, y, w - 1, h - 1); 
			gtk_gc_release(gc);
		}
	}

	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(d, style, x, y, w, h, st, "button");
}
			
static void style_handle(GB_DRAW *d, int x, int y, int w, int h, int vertical, int state)
{
	GtkStateType st = get_state(state);
	GtkStyle *style = DR(d)->style();

	gtk_paint_handle(style, DR(d)->drawable(), st,
		GTK_SHADOW_NONE, get_area(d), NULL, NULL,
		x, y, w, h,
		(!vertical) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
	if (DR(d)->mask())
		gtk_paint_handle(style, DR(d)->mask(), st, 
			GTK_SHADOW_NONE, get_area(d), NULL, NULL,
			x, y, w, h,
			(!vertical) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
}

static void style_box(GB_DRAW *d, int x, int y, int w, int h, int state)
{
	GtkStateType st = get_state(state);
	GtkStyle *style = DR(d)->style("GtkEntry", GTK_TYPE_ENTRY);

	gtk_paint_shadow(style, DR(d)->drawable(), st,
		GTK_SHADOW_IN, get_area(d), NULL, "entry", x, y, w, h);
	if (DR(d)->mask())
		gtk_paint_shadow(style, DR(d)->mask(), st,
			GTK_SHADOW_IN, get_area(d), NULL, "entry", x, y, w, h);

	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(d, style, x, y, w, h, st, "entry");
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
		style_button,
		style_panel,
		style_handle,
		style_box
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




