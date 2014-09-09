/***************************************************************************

  CStyle.cpp

  (c) 2013-2014 - Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CSTYLE_CPP

#include "CDrawingArea.h"
#include "CPicture.h"
#include "CStyle.h"
#include "cpaint_impl.h"

#ifdef GTK3

static cairo_t *_cr = NULL;

#else

static GdkDrawable *_dr = NULL;
static int _dr_x = 0;
static int _dr_y = 0;

#endif

static STYLE_T *_stl = NULL;
static GtkWidget *_widget = NULL;

#ifdef GTK3
static GtkStyleContext *get_style(GType type = G_TYPE_NONE)
{
	_stl = gt_get_style(type);

	gtk_style_context_save(_stl);

	if (type == GTK_TYPE_BUTTON)
		gtk_style_context_add_class(_stl, GTK_STYLE_CLASS_BUTTON);
	else if (type == GTK_TYPE_CHECK_BUTTON)
		gtk_style_context_add_class(_stl, GTK_STYLE_CLASS_CHECK);
	else if (type == GTK_TYPE_ENTRY)
		gtk_style_context_add_class(_stl, GTK_STYLE_CLASS_ENTRY);
	else if (type == GTK_TYPE_RADIO_BUTTON)
		gtk_style_context_add_class(_stl, GTK_STYLE_CLASS_RADIO);

	return _stl;
}
#else
static GtkStyle *attach_style(GtkStyle *style)
{
	if (_widget)
		return gtk_style_attach(style, gtk_widget_get_window(_widget));
	else
		return gtk_style_attach(style, (GdkWindow*)_dr);
}

static GtkStyle *get_style(GType type = G_TYPE_NONE)
{
	if (type == G_TYPE_NONE && _widget)
	{
		_stl = gtk_style_copy(gtk_widget_get_style(_widget));
		//_stl = gtk_style_attach(_stl, gtk_widget_get_window(_widget));
	}
	else
	{
		if (type != G_TYPE_NONE)
			_stl = gtk_style_copy(gt_get_style(type));
		else
			_stl = gtk_style_copy(gtk_widget_get_default_style());
		
		//_stl = gtk_style_attach(_stl, (GdkWindow*)_dr);
	}
	
	_stl = attach_style(_stl);
	
	return _stl;
}
#endif

#ifdef GTK3
static bool begin_draw(int *x, int *y)
{
	void *device = PAINT_get_current_device();
	if (!device)
		return TRUE;

	_cr = PAINT_get_current_context();

	if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CDRAWINGAREA *)device)->ob.widget;

		if (!(wid->cached() || wid->inDrawEvent()))
		{
			GB.Error("Cannot draw outside of 'Draw' event handler");
			return TRUE;
		}

		_widget = wid->widget;
	}
	else
	{
		_widget = NULL;
	}


	return FALSE;
}
#else
static bool begin_draw(int *x, int *y)
{
	void *device = PAINT_get_current_device();
	if (!device)
		return TRUE;
	
	cairo_t *context = PAINT_get_current_context();
	cairo_surface_flush(cairo_get_target(context));
	
	if (GB.Is(device, CLASS_DrawingArea))
	{
		gDrawingArea *wid = (gDrawingArea *)((CDRAWINGAREA *)device)->ob.widget;
		
		if (wid->cached() || wid->inDrawEvent())
		{
			if (wid->cached())
			{
				wid->resizeCache();
				_dr = wid->buffer;
			}
			else
			{
				_dr = wid->widget->window;
				GtkAllocation *a = &wid->widget->allocation;
				_dr_x = a->x;
				_dr_y = a->y;
				*x += _dr_x;
				*y += _dr_y;
			}
		}
		else
		{
			GB.Error("Cannot draw outside of 'Draw' event handler");
			return TRUE;
		}
		
		_widget = wid->widget;
	}
	else if (GB.Is(device, CLASS_Picture))
	{
		gPicture *pic = ((CPICTURE *)device)->picture;
		if (pic->isVoid())
		{
			GB.Error("Bad picture");
			return TRUE;
		}
		_dr = pic->getPixmap();
		//pic->invalidate();
		_widget = NULL;
	}
	else
	{
		GB.Error("Device not supported");
	}
	
	return FALSE;
}
#endif

static void end_draw()
{
#ifdef GTK3
	_cr = NULL;
	if (_stl)
	{
		gtk_style_context_restore(_stl);
		_stl = NULL;
	}
#else
	_dr = NULL;
	if (_stl)
	{
		gtk_style_detach(_stl);
		g_object_unref(G_OBJECT(_stl));
		_stl = NULL;
	}
#endif
	_widget = NULL;

#ifndef GTK3
	cairo_t *context = PAINT_get_current_context();
	cairo_surface_mark_dirty(cairo_get_target(context));
#endif
}

#ifdef GTK3

static STATE_T get_state(int state)
{
	int gstate = STATE_NORMAL;

	if (state & GB_DRAW_STATE_DISABLED)
		gstate |= STATE_INSENSITIVE;
	if (state & GB_DRAW_STATE_ACTIVE)
		gstate |= STATE_ACTIVE;
	if (state & GB_DRAW_STATE_HOVER)
		gstate |= STATE_PRELIGHT;
	if (state & GB_DRAW_STATE_FOCUS)
		gstate |= STATE_FOCUSED;

	return (STATE_T)gstate;
}

static void set_state(GtkStyleContext *style, int state)
{
	gtk_style_context_set_state(style, get_state(state));
}

#else

static STATE_T get_state(int state)
{
	if (state & GB_DRAW_STATE_DISABLED)
		return STATE_INSENSITIVE;
	if (state & GB_DRAW_STATE_ACTIVE)
		return STATE_ACTIVE;
	if (state & GB_DRAW_STATE_HOVER)
		return STATE_PRELIGHT;

	return STATE_NORMAL;
}

#endif

#ifndef GTK3
static GdkRectangle *get_area()
{
	static GdkRectangle area;
	
	if (PAINT_get_clip(&area.x, &area.y, &area.width, &area.height))
		return NULL;
	else
	{
		area.x += _dr_x;
		area.y += _dr_y;
		//fprintf(stderr, "clip: %d %d %d %d\n", area.x, area.y, area.width, area.height);
		return &area;
	}
}
#endif

#ifdef GTK3
static void paint_focus(STYLE_T *style, int x, int y, int w, int h)
{
	gtk_render_focus(style, _cr, x, y, w, h);
}
#else
static void paint_focus(STYLE_T *style, int x, int y, int w, int h, STATE_T state, const char *kind)
{
	gtk_paint_focus(style, _dr,
		state, get_area(), _widget, kind,
		x, y, w, h);
}
#endif

static void style_arrow(int x, int y, int w, int h, int type, int state)
{
	GtkArrowType arrow;
	STYLE_T *style = get_style(GTK_TYPE_BUTTON);
	
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
	
#ifdef GTK3
	double angle;

	switch(arrow)
	{
		case GTK_ARROW_LEFT: angle = M_PI * 1.5; break;
		case GTK_ARROW_RIGHT: angle = M_PI / 2; break;
		case GTK_ARROW_UP: angle = 0; break;
		case GTK_ARROW_DOWN: angle = M_PI; break;
		default: return;
	}

	if (w > h)
	{
		x += (w - h) / 2;
		w = h;
	}
	else if (h > w)
	{
		y += (h - w) / 2;
	}

	set_state(style, state);
	gtk_render_arrow(style, _cr, angle, x, y, w);
#else
	gtk_paint_arrow(style, _dr, get_state(state), 
		GTK_SHADOW_NONE, get_area(), _widget, NULL, 
		arrow, TRUE, x, y, w, h);
#endif
}

static void style_check(int x, int y, int w, int h, int value, int state)
{
	STYLE_T *style = get_style(GTK_TYPE_CHECK_BUTTON);

	if (value)
		state |= GB_DRAW_STATE_ACTIVE;

#ifdef GTK3
	set_state(style, state);
	gtk_render_check(style, _cr, x, y, w, h);
	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h);
#else
	GtkShadowType shadow;
	GtkStateType st = get_state(state);
	
	if (value)
		state |= GB_DRAW_STATE_ACTIVE;

	//_dr->offset(&x, &y);
	
	switch (value)
	{
		case -1: shadow = GTK_SHADOW_IN; break;
		case 1: shadow = GTK_SHADOW_ETCHED_IN; break;
		default: shadow = GTK_SHADOW_OUT; break;
	}

	gtk_paint_check(style, _dr,
		st, shadow, get_area(), NULL, "checkbutton",
		x, y, w, h);
	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h, st, "checkbutton");
#endif

}

static void style_option(int x, int y, int w, int h, int value, int state)
{
	STYLE_T *style = get_style(GTK_TYPE_RADIO_BUTTON);

	if (value)
		state |= GB_DRAW_STATE_ACTIVE;

#ifdef GTK3
	set_state(style, state);
	gtk_render_option(style, _cr, x, y, w, h);
	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h);
#else
	GtkShadowType shadow;
	GtkStateType st = get_state(state | (value ? GB_DRAW_STATE_ACTIVE : 0));
	
	shadow = value ? GTK_SHADOW_IN : GTK_SHADOW_OUT;

	gtk_paint_option(style, _dr,
		st, shadow, get_area(), NULL, "radiobutton",
		x, y, w, h);
	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h, st, "radiobutton");
#endif
			
}

static void style_separator(int x, int y, int w, int h, int vertical, int state)
{
	STYLE_T *style = get_style();

	if (vertical)
	{
#ifdef GTK3
		set_state(style, state);
		gtk_render_line(style, _cr, x + (w / 2), y, x + (w / 2), y + h - 1);
#else
		gtk_paint_vline(style, _dr,
			get_state(state), get_area(), NULL, NULL,
			y, y + h - 1, x + (w / 2));
#endif
	}
	else
	{
#ifdef GTK3
		set_state(style, state);
		gtk_render_line(style, _cr, x, y + (h / 2), x + w - 1, y + (h / 2));
#else
		gtk_paint_hline(style, _dr,
			get_state(state), get_area(), NULL, NULL,
			x, x + w - 1, y + (h / 2));
#endif
	}
}

static void style_button(int x, int y, int w, int h, int value, int state, int flat)
{
	STYLE_T *style = get_style(GTK_TYPE_BUTTON);

	if (value)
		state |= GB_DRAW_STATE_ACTIVE;

#ifndef GTK3
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
#endif
	
	if (flat && (state & GB_DRAW_STATE_HOVER) == 0)
	{
		/*gtk_paint_flat_box(style, _dr,
			st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
			get_area(d), _widget, "button",
			x, y, w, h);
		if (_dr->mask())
			gtk_paint_flat_box(style, _dr->mask(),
				st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
				get_area(d), _widget, "button",
				x, y, w, h);*/
	}
	else
	{
#ifdef GTK3
		set_state(style, state);
		gtk_render_background(style, _cr, x, y, w, h);
		gtk_render_frame(style, _cr, x, y, w, h);
		if (state & GB_DRAW_STATE_FOCUS)
			paint_focus(style, x, y, w, h);
#else
	GtkStateType st = get_state(state);
		gtk_paint_box(style, _dr,
			st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
			get_area(), _widget, "button",
			x, y, w, h);
	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, xf, yf, wf, hf, st, "button");
#endif
	}

}
			
static void style_panel(int x, int y, int w, int h, int border, int state)
{
	STYLE_T *style = get_style();

#ifdef GTK3
	gColor col = 0;

	if (border == BORDER_PLAIN)
	{
		col = IMAGE.MergeColor(gDesktop::bgColor(), gDesktop::fgColor(), 0.5);
		col = IMAGE.LighterColor(col);
	}

	gt_draw_border(_cr, style, get_state(state), border, col, x, y, w, h);

#else
	GtkShadowType shadow;
	GtkStateType st = get_state(state);

	switch (border)
	{
		case BORDER_SUNKEN: shadow = GTK_SHADOW_IN; break;
		case BORDER_RAISED: shadow = GTK_SHADOW_OUT; break;
		case BORDER_ETCHED: shadow = GTK_SHADOW_ETCHED_IN; break;
		default: shadow = GTK_SHADOW_NONE;
	}
	
	gtk_paint_shadow(style, _dr, st, shadow, get_area(), NULL, NULL, x, y, w, h);

	if (border == BORDER_PLAIN)
	{
		GdkGC *gc;
		GdkGCValues values;
		uint col;

		col = IMAGE.MergeColor(gDesktop::bgColor(), gDesktop::fgColor(), 0.5);
		col = IMAGE.LighterColor(col);

		fill_gdk_color(&values.foreground, col, gdk_drawable_get_colormap(_dr));
		gc = gtk_gc_get(gdk_drawable_get_depth(_dr), gdk_drawable_get_colormap(_dr), &values, GDK_GC_FOREGROUND);
		gdk_draw_rectangle(_dr, gc, FALSE, x, y, w - 1, h - 1);
		gtk_gc_release(gc);
	}

	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h, st, "button");
#endif

}
			
static void style_handle(int x, int y, int w, int h, int vertical, int state)
{
	STYLE_T *style = get_style();

#ifdef GTK3
	set_state(style, state);
	gtk_render_handle(style, _cr, x, y, w, h);
#else
	gtk_paint_handle(style, _dr, get_state(state),
		GTK_SHADOW_NONE, get_area(), NULL, NULL,
		x, y, w, h,
		(!vertical) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
#endif
}

static void style_box(int x, int y, int w, int h, int state, GB_COLOR color)
{
	STYLE_T *style = get_style(GTK_TYPE_ENTRY);
#ifdef GTK3
	bool oxygen = false;
#endif

	if (strcmp(gApplication::getStyleName(), "oxygen-gtk") == 0)
	{
		x -= 3;
		w += 6;
#ifdef GTK3
		oxygen = true;
#endif
	}

	#ifdef GTK3
	static const char *_name = NULL;
	static const char *_bg_names[] = { "base_color", "theme_base_color", NULL };

	set_state(style, state);
	if (color != GB_COLOR_DEFAULT)
	{
		char buffer[256];
		int alpha = ((color >> 24) ^ 0xFF);

		if (oxygen)
		{
			if (alpha == 255)
				sprintf(buffer, "GtkEntry { background-color: #%06x; }", color);
			else
				sprintf(buffer, "GtkEntry { background-color: alpha(#%06x, 0.%03i); }", color & 0xFFFFFF, alpha * 1000 / 255);
		}
		else
		{
			if (!_name)
			{
				GdkRGBA rgba;
				GtkWidget *entry = gtk_entry_new();
				gt_style_lookup_color(gtk_widget_get_style_context(entry), _bg_names, &_name, &rgba);
				//fprintf(stderr, "name = %s\n", _name);
				gtk_widget_destroy(entry);
			}

			if (alpha == 255)
				sprintf(buffer, "@define-color %s #%06x;", _name, color);
			else
				sprintf(buffer, "@define-color %s alpha(#%06x, 0.%03i);", _name, color & 0xFFFFFF, alpha * 1000 / 255);
		}

		//fprintf(stderr, "style = %p buffer = %s\n", style, buffer);
		GtkCssProvider *provider = gtk_css_provider_new();
		gtk_css_provider_load_from_data(provider, buffer, strlen(buffer), NULL);
		gtk_style_context_add_provider(style, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
		gtk_style_context_invalidate(style);
		gtk_render_background(style, _cr, x, y, w, h);
		gtk_style_context_remove_provider(style, GTK_STYLE_PROVIDER(provider));
		gtk_style_context_invalidate(style);
		g_object_unref(G_OBJECT(provider));
	}

	gtk_render_frame(style, _cr, x, y, w, h);
#else
	GtkStateType st = get_state(state);

	if (color == GB_COLOR_DEFAULT)
	{
		gtk_paint_shadow(style, _dr, st,
			GTK_SHADOW_IN, get_area(), NULL, "entry", x, y, w, h);

		/*color = gDesktop::bgColor();
		if (_widget)
		{
			gControl *control = gt_get_control(_widget);
			if (control)
				color = control->realBackground();
		}*/
	}
	else
	{
		GtkStyle *style2 = gtk_style_copy(style);
		for (int i = 0; i < 5; i++)
		{
			fill_gdk_color(&style2->bg[i], color);
			fill_gdk_color(&style2->base[i], color);
		}
		style2 = attach_style(style2);

		gtk_paint_box (style2, _dr, st,
			GTK_SHADOW_IN, get_area(), _widget, "entry", x, y, w, h);

		g_object_unref(G_OBJECT(style2));
	}

	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h, st, "entry");

#endif
}

BEGIN_PROPERTY(Style_ScrollbarSize)

	GB.ReturnInteger(gApplication::getScrollbarSize());

END_PROPERTY

BEGIN_PROPERTY(Style_ScrollbarSpacing)

	GB.ReturnInteger(gApplication::getScrollbarSpacing());

END_PROPERTY

BEGIN_PROPERTY(Style_FrameWidth)

	GB.ReturnInteger(gApplication::getFrameWidth());

END_PROPERTY

BEGIN_PROPERTY(Style_BoxFrameWidth)

	int w, h;
	gApplication::getBoxFrame(&w, &h);
	GB.ReturnInteger(w);

END_PROPERTY

BEGIN_PROPERTY(Style_BoxFrameHeight)

	int w, h;
	gApplication::getBoxFrame(&w, &h);
	GB.ReturnInteger(h);

END_PROPERTY

BEGIN_PROPERTY(Style_Name)

	GB.ReturnNewZeroString(gApplication::getStyleName());

END_PROPERTY

#define BEGIN_DRAW() \
	int x, y, w, h; \
\
	x = VARG(x); \
	y = VARG(y); \
	w = VARG(w); \
	h = VARG(h); \
\
	if (w < 1 || h < 1) \
		return; \
		\
	if (begin_draw(&x, &y)) \
		return;

#define END_DRAW() end_draw()
	
BEGIN_METHOD(Style_PaintArrow, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER type; GB_INTEGER state)

	BEGIN_DRAW();
	style_arrow(x, y, w, h, VARG(type), VARGOPT(state, GB_DRAW_STATE_NORMAL));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_PaintCheck, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER value; GB_INTEGER state)

	BEGIN_DRAW();
	style_check(x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_PaintOption, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN value; GB_INTEGER state)

	BEGIN_DRAW();
	style_option(x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_PaintSeparator, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN vertical; GB_INTEGER state)

	BEGIN_DRAW();
	style_separator(x, y, w, h, VARGOPT(vertical, FALSE), VARGOPT(state, GB_DRAW_STATE_NORMAL));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_PaintButton, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN value; GB_INTEGER state; GB_BOOLEAN flat)

	BEGIN_DRAW();
	style_button(x, y, w, h, VARG(value), VARGOPT(state, GB_DRAW_STATE_NORMAL), VARGOPT(flat, FALSE));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_PaintPanel, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER border; GB_INTEGER state)

	BEGIN_DRAW();
	style_panel(x, y, w, h, VARG(border), VARGOPT(state, GB_DRAW_STATE_NORMAL));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_PaintHandle, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_BOOLEAN vertical; GB_INTEGER state)

	BEGIN_DRAW();
	style_handle(x, y, w, h, VARGOPT(vertical, FALSE), VARGOPT(state, GB_DRAW_STATE_NORMAL));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_PaintBox, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER state; GB_INTEGER color)

	BEGIN_DRAW();
	style_box(x, y, w, h, VARGOPT(state, GB_DRAW_STATE_NORMAL), VARGOPT(color, GB_COLOR_DEFAULT));
	END_DRAW();

END_METHOD

BEGIN_METHOD(Style_StateOf, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);
	gControl *widget;
	int state;
	bool design;

	if (GB.CheckObject(control))
		return;

	widget = control->widget;
	state = GB_DRAW_STATE_NORMAL;
	design = widget->design();

	if (!widget->isEnabled())
		state |= GB_DRAW_STATE_DISABLED;
	if (widget->hasVisibleFocus() && !design)
		state |= GB_DRAW_STATE_FOCUS;
	if (widget->hovered() && !design)
		state |= GB_DRAW_STATE_HOVER;

	GB.ReturnInteger(state);

END_METHOD

BEGIN_METHOD(Style_BackgroundOf, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);

	if (GB.CheckObject(control))
		return;

	GB.ReturnInteger(control->widget->realBackground(true));

END_METHOD

BEGIN_METHOD(Style_ForegroundOf, GB_OBJECT control)

	CWIDGET *control = (CWIDGET *)VARG(control);

	if (GB.CheckObject(control))
		return;

	GB.ReturnInteger(control->widget->realForeground(true));

END_METHOD


GB_DESC StyleDesc[] =
{
	GB_DECLARE("Style", 0), GB_VIRTUAL_CLASS(),
	
	GB_STATIC_PROPERTY_READ("ScrollbarSize", "i", Style_ScrollbarSize),
	GB_STATIC_PROPERTY_READ("ScrollbarSpacing", "i", Style_ScrollbarSpacing),
	GB_STATIC_PROPERTY_READ("FrameWidth", "i", Style_FrameWidth),
	GB_STATIC_PROPERTY_READ("TextBoxFrameWidth", "i", Style_BoxFrameWidth),
	GB_STATIC_PROPERTY_READ("BoxFrameWidth", "i", Style_BoxFrameWidth),
	GB_STATIC_PROPERTY_READ("BoxFrameHeight", "i", Style_BoxFrameHeight),
	GB_STATIC_PROPERTY_READ("Name", "s", Style_Name),
	
	GB_STATIC_METHOD("PaintArrow", NULL, Style_PaintArrow, "(X)i(Y)i(Width)i(Height)i(Type)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintCheck", NULL, Style_PaintCheck, "(X)i(Y)i(Width)i(Height)i(Value)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintOption", NULL, Style_PaintOption, "(X)i(Y)i(Width)i(Height)i(Value)b[(Flag)i]"),
	GB_STATIC_METHOD("PaintSeparator", NULL, Style_PaintSeparator, "(X)i(Y)i(Width)i(Height)i[(Vertical)b(Flag)i]"),
	GB_STATIC_METHOD("PaintButton", NULL, Style_PaintButton, "(X)i(Y)i(Width)i(Height)i(Value)b[(Flag)i(Flat)b]"),
	GB_STATIC_METHOD("PaintPanel", NULL, Style_PaintPanel, "(X)i(Y)i(Width)i(Height)i(Border)i[(Flag)i]"),
	GB_STATIC_METHOD("PaintHandle", NULL, Style_PaintHandle, "(X)i(Y)i(Width)i(Height)i[(Vertical)b(Flag)i]"),
	GB_STATIC_METHOD("PaintBox", NULL, Style_PaintBox, "(X)i(Y)i(Width)i(Height)i[(Flag)i(Color)i]"),
	
	GB_CONSTANT("Normal", "i", GB_DRAW_STATE_NORMAL),
	GB_CONSTANT("Disabled", "i", GB_DRAW_STATE_DISABLED),
	GB_CONSTANT("HasFocus", "i", GB_DRAW_STATE_FOCUS),
	GB_CONSTANT("Hovered", "i", GB_DRAW_STATE_HOVER),
	GB_CONSTANT("Active", "i", GB_DRAW_STATE_ACTIVE),

	GB_STATIC_METHOD("StateOf", "i", Style_StateOf, "(Control)Control;"),
	GB_STATIC_METHOD("BackgroundOf", "i", Style_BackgroundOf, "(Control)Control;"),
	GB_STATIC_METHOD("ForegroundOf", "i", Style_ForegroundOf, "(Control)Control;"),

	GB_END_DECLARE
};

