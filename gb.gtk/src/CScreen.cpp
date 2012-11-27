/***************************************************************************

  CScreen.cpp

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __CSCREEN_CPP

#include "CWindow.h"
#include "CPicture.h"
#include "CFont.h"
#include "CDrawingArea.h"
#include "CScreen.h"

#include "gtrayicon.h"
#include "gapplication.h"
#include "gmainwindow.h"
#include "gdraw.h"
#include "cpaint_impl.h"

extern int CWINDOW_Embedder;
extern bool CWINDOW_Embedded;

extern int MAIN_scale;

char *CAPPLICATION_Theme = 0;

static int _busy = 0;

#define MAX_SCREEN 16
static CSCREEN *_screens[MAX_SCREEN] = { NULL };

static CSCREEN *get_screen(int num)
{
	if (num < 0 || num >= MAX_SCREEN || num >= gDesktop::count())
		return NULL;
	
	if (!_screens[num])
	{
		_screens[num] = (CSCREEN *)GB.New(GB.FindClass("Screen"), NULL, 0);
		_screens[num]->index = num;
		GB.Ref(_screens[num]);
	}
	
	return _screens[num];
}

static void free_screens(void)
{
	int i;
	
	for (i = 0; i < MAX_SCREEN; i++)
	{
		if (_screens[i])
			GB.Unref(POINTER(&_screens[i]));
	}
}

static GdkRectangle *geometry(int num)
{
	static GdkRectangle rect;
	gDesktop::geometry(num, &rect);
	return &rect;
}

static GdkRectangle *available_geometry(int num)
{
	static GdkRectangle rect;
	gDesktop::availableGeometry(num, &rect);
	return &rect;
}

BEGIN_PROPERTY(Desktop_X)

	GB.ReturnInteger(available_geometry(0)->x);

END_PROPERTY

BEGIN_PROPERTY(Desktop_Y)

	GB.ReturnInteger(available_geometry(0)->y);

END_PROPERTY

BEGIN_PROPERTY(Desktop_Width)

	GB.ReturnInteger(available_geometry(0)->width);

END_PROPERTY

BEGIN_PROPERTY(Desktop_Height)

	GB.ReturnInteger(available_geometry(0)->height);

END_PROPERTY

BEGIN_PROPERTY(Desktop_Resolution)

	GB.ReturnInteger(gDesktop::resolution());

END_PROPERTY

BEGIN_METHOD(Desktop_Screenshot, GB_INTEGER x; GB_INTEGER y; GB_INTEGER width; GB_INTEGER height)

	CPICTURE *pic;
	gPicture *buf = gDesktop::screenshot(VARGOPT(x,0), VARGOPT(y, 0), VARGOPT(width, 0), VARGOPT(height, 0));
	
	pic = (CPICTURE *)GB.New(GB.FindClass("Picture"), 0, 0);
	if (pic->picture) pic->picture->unref();
	pic->picture = buf;
	GB.ReturnObject(pic);

END_METHOD

BEGIN_PROPERTY(Desktop_HasSystemTray)

	#ifdef NO_X_WINDOW
		GB.Return(FALSE);
	#else
		GB.ReturnBoolean(gTrayIcon::hasSystemTray());
	#endif

END_PROPERTY


static void set_font(gFont *font, void *object = 0)
{
	gDesktop::setFont(font);
	MAIN_scale = gDesktop::scale();
}

BEGIN_PROPERTY(Application_Font)

	if (READ_PROPERTY)
		GB.ReturnObject(CFONT_create(gDesktop::font()->copy(), set_font));
	else if (VPROP(GB_OBJECT))
		set_font(((CFONT*)VPROP(GB_OBJECT))->font);

END_PROPERTY


BEGIN_PROPERTY(Application_ActiveWindow)

	GB.ReturnObject(CWINDOW_Active);

END_PROPERTY


BEGIN_PROPERTY(Application_ActiveControl)

	GB.ReturnObject(GetObject(gApplication::activeControl()));

END_PROPERTY


BEGIN_PROPERTY(Application_Busy)

	int busy;

	if (READ_PROPERTY)
		GB.ReturnInteger(_busy);
	else
	{
		busy = VPROP(GB_INTEGER);

		if (_busy == 0 && busy != 0)
			gApplication::setBusy(true);
		else if (_busy > 0 && busy == 0)
			gApplication::setBusy(false);

		_busy = busy;
		if (MAIN_debug_busy)
			fprintf(stderr, "%s: Application.Busy = %d\n", GB.Debug.GetCurrentPosition(), busy);
	}

END_PROPERTY


BEGIN_PROPERTY(Desktop_Scale)

	GB.ReturnInteger(MAIN_scale);

END_PROPERTY


BEGIN_PROPERTY(Application_ShowTooltips)

	if (READ_PROPERTY)
		GB.ReturnBoolean(gApplication::areTooltipsEnabled());
	else
		gApplication::enableTooltips(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(Application_MainWindow)

	if (READ_PROPERTY)
		GB.ReturnObject(CWINDOW_Main);
	else
	{
		CWINDOW_Main = (CWINDOW *)VPROP(GB_OBJECT);
		gApplication::setMainWindow(CWINDOW_Main ? (gMainWindow *)CWINDOW_Main->ob.widget : NULL);
	}

END_PROPERTY


BEGIN_METHOD_VOID(Application_exit)

	GB.FreeString(&CAPPLICATION_Theme);
	free_screens();

END_METHOD


BEGIN_PROPERTY(Application_Embedder)

	if (READ_PROPERTY)
		GB.ReturnInteger(CWINDOW_Embedder);
	else
	{
		if (CWINDOW_Embedded)
		{
			GB.Error("Application is already embedded");
			return;
		}
	
		CWINDOW_Embedder = VPROP(GB_INTEGER);
	}

END_PROPERTY


BEGIN_PROPERTY(Application_Theme)

	if (READ_PROPERTY) { GB.ReturnString(CAPPLICATION_Theme); return; }
	GB.StoreString(PROP(GB_STRING), &CAPPLICATION_Theme);

END_PROPERTY


BEGIN_PROPERTY(Screens_Count)

	GB.ReturnInteger(gDesktop::count());

END_PROPERTY


BEGIN_METHOD(Screens_get, GB_INTEGER screen)

	GB.ReturnObject(get_screen(VARG(screen)));

END_METHOD


BEGIN_METHOD_VOID(Screens_next)

	int *index = (int *)GB.GetEnum();

	if (*index >= gDesktop::count())
		GB.StopEnum();
	else
	{
		GB.ReturnObject(get_screen(*index));
		(*index)++;
	}
	
END_METHOD


BEGIN_PROPERTY(Screen_X)

	GB.ReturnInteger(geometry(SCREEN->index)->x);

END_PROPERTY

BEGIN_PROPERTY(Screen_Y)

	GB.ReturnInteger(geometry(SCREEN->index)->y);

END_PROPERTY

BEGIN_PROPERTY(Screen_Width)

	GB.ReturnInteger(geometry(SCREEN->index)->width);

END_PROPERTY

BEGIN_PROPERTY(Screen_Height)

	GB.ReturnInteger(geometry(SCREEN->index)->height);

END_PROPERTY


BEGIN_PROPERTY(Screen_AvailableX)

	GB.ReturnInteger(available_geometry(SCREEN->index)->x);

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableY)

	GB.ReturnInteger(available_geometry(SCREEN->index)->y);

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableWidth)

	GB.ReturnInteger(available_geometry(SCREEN->index)->width);

END_PROPERTY

BEGIN_PROPERTY(Screen_AvailableHeight)

	GB.ReturnInteger(available_geometry(SCREEN->index)->height);

END_PROPERTY


GB_DESC ScreenDesc[] =
{
	GB_DECLARE("Screen", sizeof(CSCREEN)), GB_NOT_CREATABLE(), GB_AUTO_CREATABLE(),

	GB_PROPERTY_READ("X", "i", Screen_X),
	GB_PROPERTY_READ("Y", "i", Screen_Y),
	GB_PROPERTY_READ("W", "i", Screen_Width),
	GB_PROPERTY_READ("H", "i", Screen_Height),
	GB_PROPERTY_READ("Width", "i", Screen_Width),
	GB_PROPERTY_READ("Height", "i", Screen_Height),

	GB_PROPERTY_READ("AvailableX", "i", Screen_AvailableX),
	GB_PROPERTY_READ("AvailableY", "i", Screen_AvailableY),
	GB_PROPERTY_READ("AvailableWidth", "i", Screen_AvailableWidth),
	GB_PROPERTY_READ("AvailableHeight", "i", Screen_AvailableHeight),

	GB_END_DECLARE
};

GB_DESC ScreensDesc[] =
{
	GB_DECLARE("Screens", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("Count", "i", Screens_Count),
	GB_STATIC_METHOD("_get", "Screen", Screens_get, "(Screen)i"),
	GB_STATIC_METHOD("_next", "Screen", Screens_next, NULL),
	
	GB_END_DECLARE
};

GB_DESC DesktopDesc[] =
{
	GB_DECLARE("Desktop", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("X", "i", Desktop_X),
	GB_STATIC_PROPERTY_READ("Y", "i", Desktop_Y),
	GB_STATIC_PROPERTY_READ("W", "i", Desktop_Width),
	GB_STATIC_PROPERTY_READ("H", "i", Desktop_Height),
	GB_STATIC_PROPERTY_READ("Width", "i", Desktop_Width),
	GB_STATIC_PROPERTY_READ("Height", "i", Desktop_Height),

	GB_CONSTANT("Charset", "s", "UTF-8"),
	GB_STATIC_PROPERTY_READ("Resolution", "i", Desktop_Resolution),
	GB_STATIC_PROPERTY_READ("Scale","i",Desktop_Scale),
	GB_STATIC_PROPERTY_READ("HasSystemTray", "b", Desktop_HasSystemTray),
	
	GB_STATIC_METHOD("Screenshot", "Picture", Desktop_Screenshot, "[(X)i(Y)i(Width)i(Height)i]"),

	GB_END_DECLARE
};

GB_DESC ApplicationDesc[] =
{
	GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("_exit", NULL, Application_exit, 0),

	GB_STATIC_PROPERTY("Font", "Font", Application_Font),
	GB_STATIC_PROPERTY_READ("ActiveControl","Control",Application_ActiveControl),
	GB_STATIC_PROPERTY_READ("ActiveWindow", "Window", Application_ActiveWindow),
	GB_STATIC_PROPERTY("MainWindow", "Window", Application_MainWindow),
	GB_STATIC_PROPERTY("Busy", "i", Application_Busy),
	GB_STATIC_PROPERTY("ShowTooltips", "b", Application_ShowTooltips),
	
	GB_STATIC_PROPERTY("Embedder", "i", Application_Embedder),
	GB_STATIC_PROPERTY("Theme", "s", Application_Theme),

	GB_END_DECLARE
};


//-- Style ------------------------------------------------------------------

static GdkDrawable *_dr = NULL;
static GtkWidget *_widget = NULL;
static GtkStyle *_stl = NULL;
//static char *_stl_name = NULL;

static GtkStyle *get_style(const char *name = NULL, GType type = G_TYPE_NONE)
{
	//if (stl && (!name || (stl_name && !strcmp(name, stl_name))))
	//	return stl;
	
	/*if (stl)
	{
		g_object_unref(stl);
		stl = NULL;
	}*/
	
	if (!name && _widget)
	{
		_stl = gtk_style_copy(_widget->style);
		_stl = gtk_style_attach(_stl, _widget->window);
	}
	else
	{
		if (name)
			_stl = gtk_style_copy(gt_get_style(name, type));
		else
			_stl = gtk_style_copy(gtk_widget_get_default_style());
		
		_stl = gtk_style_attach(_stl, (GdkWindow*)_dr);
	}
	
	//stl_name = name;
	
	return _stl;
}

static bool begin_draw()
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
				//GtkAllocation *a = &wid->widget->allocation;
				//_x = a->x;
				//_y = a->y;
			}
		}
		else
		{
			GB.Error("Cannot draw outside of Draw event handler");
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

static void end_draw()
{
	_dr = NULL;
	_widget = NULL;
	if (_stl)
	{
		g_object_unref(G_OBJECT(_stl));
		_stl = NULL;
	}

	cairo_t *context = PAINT_get_current_context();
	cairo_surface_mark_dirty(cairo_get_target(context));
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

static GdkRectangle *get_area()
{
	static GdkRectangle area;
	
	return NULL;
	/*area.x = -600;
	area.y = -600;
	area.width = 1024;
	area.height = 1024;
	return &area;*/
	
	if (PAINT_get_clip(&area.x, &area.y, &area.width, &area.height))
		return NULL;
	else
	{
		//_dr->offset(&area.x, &area.y);
		fprintf(stderr, "clip: %d %d %d %d\n", area.x, area.y, area.width, area.height);
		return &area;
	}
}

/*static GtkWidget *get_widget(GB_DRAW *d)
{
	if (!GB.Is(d->device, CLASS_DrawingArea))
		return NULL;
	
	return ((gDrawingArea *)((CDRAWINGAREA *)d->device)->ob.widget)->border;
}*/

static void paint_focus(GtkStyle *style, int x, int y, int w, int h, GtkStateType state, const char *kind)
{
	gtk_paint_focus(style, _dr,
		state, 
		get_area(), _widget, kind, 
		x, y, w, h);
}

static void style_arrow(int x, int y, int w, int h, int type, int state)
{
	GtkArrowType arrow;
	GtkStyle *style = get_style();
	
	PAINT_apply_offset(&x, &y);
	
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
	
	gtk_paint_arrow(style, _dr, get_state(state), 
		GTK_SHADOW_NONE, get_area(), _widget, NULL, 
		arrow, TRUE, x, y, w, h);
	
	fprintf(stderr, "style_arrow: %d %d %d %d\n", x, y, w, h);
}

static void style_check(int x, int y, int w, int h, int value, int state)
{
	GtkShadowType shadow;
	GtkStateType st = get_state(state | (value ? GB_DRAW_STATE_ACTIVE : 0));
	GtkStyle *style = get_style("GtkCheckButton", GTK_TYPE_CHECK_BUTTON);
	
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
}

static void style_option(int x, int y, int w, int h, int value, int state)
{
	GtkShadowType shadow;
	GtkStateType st = get_state(state | (value ? GB_DRAW_STATE_ACTIVE : 0));
	GtkStyle *style = get_style("GtkRadioButton", GTK_TYPE_RADIO_BUTTON);
	
	//_dr->offset(&x, &y);
	
	shadow = value ? GTK_SHADOW_IN : GTK_SHADOW_OUT;

	gtk_paint_option(style, _dr,
		st, shadow, get_area(), NULL, "radiobutton", 
		x, y, w, h);
			
	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h, st, "radiobutton");
}

static void style_separator(int x, int y, int w, int h, int vertical, int state)
{
	GtkStateType st = get_state(state);
	GtkStyle *style = get_style();
	
	//_dr->offset(&x, &y);
	
	if (vertical)
	{
		gtk_paint_vline(style, _dr,
			st, get_area(), NULL, NULL, 
			y, y + h - 1, x + (w / 2));
	}
	else
	{
		gtk_paint_hline(style, _dr,
			st, get_area(), NULL, NULL, 
			x, x + w - 1, y + (h / 2));
	}
}

static void style_button(int x, int y, int w, int h, int value, int state, int flat)
{
	GtkStateType st = get_state(state | (value ? GB_DRAW_STATE_ACTIVE : 0));
	GtkStyle *style = get_style("GtkButton", GTK_TYPE_BUTTON);
	int xf, yf, wf, hf;
	GtkBorder *default_border, *default_outside_border, *inner_border;
	int focus_width, focus_pad, df;
	gboolean interior_focus;
	
	//_dr->offset(&x, &y);
	
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
		fprintf(stderr, "paint box: %d %d %d %d\n", x, y, w, h);
		gtk_paint_box(style, _dr,
			st, value ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
			get_area(), _widget, "button",
			x, y, w, h);
	}

	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, xf, yf, wf, hf, st, "button");
}
			
static void style_panel(int x, int y, int w, int h, int border, int state)
{
	GtkShadowType shadow;
	GtkStateType st = get_state(state);
	GtkStyle *style = get_style();

	PAINT_apply_offset(&x, &y);
	
	switch (border)
	{
		case BORDER_SUNKEN: shadow = GTK_SHADOW_IN; break;
		case BORDER_RAISED: shadow = GTK_SHADOW_OUT; break;
		case BORDER_ETCHED: shadow = GTK_SHADOW_ETCHED_IN; break;
		default: shadow = GTK_SHADOW_NONE;
	}
	
	gtk_paint_shadow(style, _dr, 
		st, shadow, get_area(), NULL, NULL, 
		x, y, w, h);
	
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
}
			
static void style_handle(int x, int y, int w, int h, int vertical, int state)
{
	GtkStateType st = get_state(state);
	GtkStyle *style = get_style();

	//_dr->offset(&x, &y);
	
	gtk_paint_handle(style, _dr, st,
		GTK_SHADOW_NONE, get_area(), NULL, NULL,
		x, y, w, h,
		(!vertical) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);
}

static void style_box(int x, int y, int w, int h, int state)
{
	//static GtkWidget *widget = NULL;
	
	GtkStateType st = get_state(state);
	GtkStyle *style = get_style("GtkEntry", GTK_TYPE_ENTRY);

	//_dr->offset(&x, &y);
	
	//if (!widget)
		//widget = gtk_entry_new();
	
	gtk_paint_shadow(style, _dr, st,
		GTK_SHADOW_IN, get_area(), NULL, "entry", x, y, w, h);

	if (state & GB_DRAW_STATE_FOCUS)
		paint_focus(style, x, y, w, h, st, "entry");
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
	if (begin_draw()) \
		return; \

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

BEGIN_METHOD(Style_PaintBox, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER state)

	BEGIN_DRAW();
	style_box(x, y, w, h, VARGOPT(state, GB_DRAW_STATE_NORMAL));
	END_DRAW();

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
	GB_STATIC_METHOD("PaintBox", NULL, Style_PaintBox, "(X)i(Y)i(Width)i(Height)i[(Flag)i]"),
	
	GB_END_DECLARE
};

