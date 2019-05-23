/***************************************************************************

	gcontrol.cpp

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

#include <unistd.h>

#ifndef GAMBAS_DIRECTFB
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/X.h>
#endif
#endif

#include "widgets.h"
#include "gapplication.h"
#include "gbutton.h"
#include "gdrawingarea.h"
#include "gmainwindow.h"
#include "gmoviebox.h"
#include "gplugin.h"
#include "gscrollbar.h"
#include "gslider.h"
#include "gdesktop.h"
#include "gdrag.h"
#include "gmouse.h"
#include "gcontrol.h"

static GList *controls = NULL;
static GList *controls_destroyed = NULL;

#ifdef GTK3

typedef
	struct {
		void (*get_preferred_height)(GtkWidget *widget, gint *minimum_height, gint *natural_height);
		void (*get_preferred_width_for_height)(GtkWidget *widget, gint height, gint *minimum_width, gint *natural_width);
		void (*get_preferred_width)(GtkWidget *widget, gint *minimum_width, gint *natural_width);
		void (*get_preferred_height_for_width)(GtkWidget *widget, gint width, gint *minimum_height, gint *natural_height);
#if GTK_CHECK_VERSION(3, 10, 0)
		void (*get_preferred_height_and_baseline_for_width)(GtkWidget *widget, gint width, gint *minimum, gint *natural, gint *minimum_baseline, gint *natural_baseline);
		void (*size_allocate)(GtkWidget *widget, GtkAllocation *allocation);
#endif
	}
	PATCH_FUNCS;
	
#endif
	
#if 0
static const char *_cursor_fdiag[] =
{
"16 16 4 1",
"# c None",
"a c #000000",
"b c #c0c0c0",
". c #ffffff",
"..........######",
".aaaaaaaa.######",
".a....ba.#######",
".a...ba.########",
".a....ab.#######",
".a.b...ab.######",
".abaa...ab.###..",
".aa.ba...ab.#.a.",
".a.#.ba...ab.aa.",
"..###.ba...aaba.",
"######.ba...b.a.",
"#######.ba....a.",
"########.ab...a.",
"#######.ab....a.",
"######.aaaaaaaa.",
"######.........."
};

static const char *_cursor_bdiag[] =
{
"16 16 4 1",
". c None",
"a c #000000",
"b c #c0c0c0",
"# c #ffffff",
"......##########",
"......#aaaaaaaa#",
".......#ab####a#",
"........#ab###a#",
".......#ba####a#",
"......#ba###b#a#",
"##...#ba###aaba#",
"#a#.#ba###ab#aa#",
"#aa#ba###ab#.#a#",
"#abaa###ab#...##",
"#a#b###ab#......",
"#a####ab#.......",
"#a###ba#........",
"#a####ba#.......",
"#aaaaaaaa#......",
"##########......"
};
#endif

// Geometry optimization hack - Sometimes fails, so it is disabled...
#define GEOMETRY_OPTIMIZATION 0


#ifdef GTK3
static gboolean cb_frame_draw(GtkWidget *wid, cairo_t *cr, gControl *control)
{
	control->drawBorder(cr);
	return false;
}
#else
static gboolean cb_frame_expose(GtkWidget *wid, GdkEventExpose *e, gControl *control)
{
	control->drawBorder(e);
	return false;
}
#endif

#ifdef GTK3
static gboolean cb_background_draw(GtkWidget *wid, cairo_t *cr, gControl *control)
{
	control->drawBackground(cr);
	return false;
}
#else
static gboolean cb_background_expose(GtkWidget *wid, GdkEventExpose *e, gControl *control)
{
	control->drawBackground(e);
	return false;
}
#endif


/****************************************************************************

gPlugin

****************************************************************************/

gboolean gPlugin_OnUnplug(GtkSocket *socket,gPlugin *data)
{
	if (data->onUnplug) data->onUnplug(data);
	return true;
}

void gPlugin_OnPlug(GtkSocket *socket,gPlugin *data)
{
	if (data->onPlug) data->onPlug(data);
}


gPlugin::gPlugin(gContainer *parent) : gControl(parent)
{
	g_typ=Type_gPlugin;

	border = gtk_socket_new();
	widget = border;
	realize(false);

	onPlug = NULL;
	onUnplug = NULL;

	g_signal_connect(G_OBJECT(widget), "plug-removed", G_CALLBACK(gPlugin_OnUnplug), (gpointer)this);
	g_signal_connect(G_OBJECT(widget), "plug-added", G_CALLBACK(gPlugin_OnPlug), (gpointer)this);

	ON_DRAW_BEFORE(border, this, cb_background_expose, cb_background_draw);

	setCanFocus(true);
}

int gPlugin::client()
{
	//GdkNativeWindow win = gtk_socket_get_id(GTK_SOCKET(widget));
	//return (long)win;
	GdkWindow *win = gtk_socket_get_plug_window(GTK_SOCKET(widget));
	if (!win)
		return 0;
	else
		return (int)GDK_WINDOW_XID(win);
}

void gPlugin::plug(int id)
{
	void (*func)(gControl *);
	int i;
	Display *d = gdk_x11_display_get_xdisplay(gdk_display_get_default());

	func = onPlug;
	onPlug = NULL;

	for (i = 1; i >= 0; i--)
	{
		if (i == 0)
			onPlug = func;

		gtk_socket_add_id(GTK_SOCKET(widget), (Window)id);
	}

	if (client())
		XAddToSaveSet(d, client());
	else
		emit(SIGNAL(onError));
}

void gPlugin::discard()
{
	#ifdef GDK_WINDOWING_X11
	if (MAIN_display_x11)
	{
		Display *d = gdk_x11_display_get_xdisplay(gdk_display_get_default());

		if (!client()) return;

		XRemoveFromSaveSet(d, client());
		XReparentWindow(d, client(), GDK_ROOT_WINDOW(), 0, 0);
	}
	#else
	stub("no-X11/gPlugin:discard()");
	#endif
}


/*****************************************************************

CREATION AND DESTRUCTION

******************************************************************/

void gControl::cleanRemovedControls()
{
	GList *iter;
	gControl *control;

	if (!controls_destroyed) return;

	for(;;)
	{
		iter = g_list_first(controls_destroyed);
		if (!iter)
			break;
		control = (gControl *)iter->data;
		gtk_widget_destroy(control->border);
	}

	controls_destroyed = NULL;
}

static bool always_can_raise(gControl *sender, int type)
{
	return true;
}

void gControl::initAll(gContainer *parent)
{
	bufW = 0;
	bufH = 0;
	bufX = 0;
	bufY = 0;
	curs = NULL;
	_font = NULL;
	_resolved_font = NULL;
	g_typ = 0;
	dsg = false;
	expa = false;
	igno = false;
	_accept_drops = false;
	_drag_enter = false;
	_drag_get_data = false;
	frame_border = 0;
	frame_padding = 0;
	_bg_set = false;
	_fg_set = false;
	have_cursor = false;
	use_base = false;
	_mouse = CURSOR_DEFAULT;
	pr = parent;
	_name = NULL;
	visible = false;
	_locked = 0;
	_destroyed = false;
	_no_delete = false;
	_action = false;
	_dirty_pos = _dirty_size = false;
	_tracking = false;
	_has_input_method = false;
	_no_default_mouse_event = false;
	_proxy = _proxy_for = NULL;
	_no_tab_focus = false;
	_inside = false;
	_no_auto_grab = false;
	_no_background = false;
	_use_wheel = false;
	_scrollbar = SCROLL_NONE;
	_input_method = NULL;
	_tooltip = NULL;

	onFinish = NULL;
	onFocusEvent = NULL;
	onKeyEvent = NULL;
	onMouseEvent = NULL;
	onDrag = NULL;
	onDragMove = NULL;
	onDrop = NULL;
	onDragLeave = NULL;
	onEnterLeave = NULL;
	canRaise = always_can_raise;

	frame = border = widget = NULL;
	_scroll = NULL;
	hFree = NULL;
	_grab = false;

	_fg = _bg = COLOR_DEFAULT;
#ifdef GTK3
	_css = NULL;
	_fg_name = _bg_name = NULL;
#endif

	controls = g_list_append(controls,this);
}

gControl::gControl()
{
	initAll(NULL);
}

gControl::gControl(gContainer *parent)
{
	initAll(parent);
}

gControl::~gControl()
{
	gMainWindow *win = window();

	emit(SIGNAL(onFinish));

	/*if (pr)
		pr->remove(this);*/
	
	if (win && win->focus == this)
		win->focus = NULL;

	if (_proxy)
		_proxy->_proxy_for = NULL;
	if (_proxy_for)
		_proxy_for->_proxy = NULL;

	if (gDrag::getSource() == this)
		gDrag::cancel();

	if (curs)
	{
		delete curs;
		curs=NULL;
	}

	if (_font)
	{
		gFont::assign(&_font);
		gFont::assign(&_resolved_font);
	}

#ifdef GTK3
	if (_css)
		g_object_unref(_css);
#endif
	
	//fprintf(stderr, "~gControl: %s\n", name());

	if (_name)
		g_free(_name);
	if (_tooltip)
		g_free(_tooltip);

	controls = g_list_remove(controls, this);
	controls_destroyed = g_list_remove(controls_destroyed, this);

	#define CLEAN_POINTER(_p) if (_p == this) _p = NULL

	CLEAN_POINTER(gApplication::_enter);
	CLEAN_POINTER(gApplication::_leave);
	CLEAN_POINTER(gApplication::_active_control);
	CLEAN_POINTER(gApplication::_previous_control);
	CLEAN_POINTER(gApplication::_old_active_control);
	CLEAN_POINTER(gApplication::_button_grab);
	CLEAN_POINTER(gApplication::_enter_after_button_grab);
	CLEAN_POINTER(gApplication::_control_grab);
	CLEAN_POINTER(gApplication::_ignore_until_next_enter);
}

void gControl::destroy()
{
	if (_destroyed)
		return;

	hide();

	//fprintf(stderr, "added to destroy list: %p\n", this);
	controls_destroyed = g_list_prepend(controls_destroyed, (gpointer)this);
	if (pr)
		pr->remove(this);
	_destroyed = true;
}


bool gControl::isEnabled() const
{
	return gtk_widget_is_sensitive(border);
}

bool gControl::isReallyVisible()
{
	if (!isTopLevel() && !topLevel()->isReallyVisible())
		return false;
	
#if GTK_CHECK_VERSION(2, 20, 0)
	return gtk_widget_get_mapped(border);
#else
	return GTK_WIDGET_MAPPED(border);
#endif
}

void gControl::setEnabled(bool vl)
{
	gtk_widget_set_sensitive(border, vl);
}

void gControl::setVisible(bool vl)
{
	if (vl == visible)
		return;

	visible = vl;

	if (vl)
	{
		if (bufW <= 0 || bufH <= 0)
			return;

		gtk_widget_show(border);
		_dirty_size = true;
		updateGeometry();
	}
	else
	{
		if (parent() && hasFocus())
			gtk_widget_child_focus(GTK_WIDGET(gtk_widget_get_toplevel(border)), GTK_DIR_TAB_FORWARD);
		if (gtk_widget_has_grab(border))
			gtk_grab_remove(border);
		gtk_widget_hide(border);
	}

	if (pr) pr->performArrange();
}

/*****************************************************************

POSITION AND SIZE

******************************************************************/

bool gControl::getScreenPos(int *x, int *y)
{
	if (!gtk_widget_get_window(border))
	{
		*x = *y = 0; // widget is not realized
		return true;
	}

	gdk_window_get_origin(gtk_widget_get_window(border), x, y);

	//fprintf(stderr, "getScreenPos: %s: %d %d: %d\n", name(), *x, *y, gtk_widget_get_has_window(border));

	#if GTK_CHECK_VERSION(2, 18, 0)
	if (!gtk_widget_get_has_window(border))
	{
		GtkAllocation a;
		gtk_widget_get_allocation(border, &a);
		*x += a.x;
		*y += a.y;
	}
	#endif

	//fprintf(stderr, "getScreenPos: --> %d %d\n", *x, *y);
	return false;
}

int gControl::screenX()
{
	int x,y;
	getScreenPos(&x, &y);
	return x;
}

int gControl::screenY()
{
	int x,y;
	getScreenPos(&x, &y);
	return y;
}

void gControl::setLeft(int l)
{
	move(l,top());
}

void gControl::setTop(int t)
{
	move(left(),t);
}

void gControl::setWidth(int w)
{
	resize(w,height());
}

void gControl::setHeight(int h)
{
	resize(width(),h);
}

static void send_configure (gControl *control)
{
	GtkWidget *widget;
	GdkEvent *event;

	widget = control->border;

#if GTK_CHECK_VERSION(2, 20, 0)
	if (!gtk_widget_get_realized(widget))
		return;
#else
	if (!GTK_WIDGET_REALIZED(widget))
		return;
#endif

// 	if (control->isWindow())
// 	 g_debug("send configure to window: %s", control->name());

	event = gdk_event_new (GDK_CONFIGURE);

	event->configure.window = NULL; //(GdkWindow *)g_object_ref(widget->window);
	event->configure.send_event = TRUE;
	event->configure.x = control->x();
	event->configure.y = control->y();
	event->configure.width = control->width();
	event->configure.height = control->height();

	gtk_widget_event (widget, event);
	gdk_event_free (event);
}

void gControl::move(int x, int y)
{
	//GtkLayout *fx;

	if (x == bufX && y == bufY)
		return;

	bufX = x;
	bufY = y;

	//g_debug("move: %p: %d %d", this, x, y);
	_dirty_pos = true;
	if (pr)
	{
		// TODO: check the following optimization to see if it can be enabled again
		//if (gtk_widget_get_parent(border) == pr->getContainer())
			pr->performArrange();
	}

	#if GEOMETRY_OPTIMIZATION
	gApplication::setDirty();
	#else
	updateGeometry();
	#endif

	send_configure(this); // needed for Watcher and Form Move events
}

void gControl::resize(int w, int h)
{
	if (w < minimumWidth())
		w = minimumWidth();

	if (h < minimumHeight())
		h = minimumHeight();

	if (bufW == w && bufH == h)
		return;

	if (w < 1 || h < 1)
	{
		bufW = w;
		bufH = h;

		if (visible)
			gtk_widget_hide(border);
	}
	else
	{
		bufW = w;
		bufH = h;

		if (frame && widget != border)
		{
			int fw = getFrameWidth() * 2;
			if (w < fw || h < fw)
				gtk_widget_hide(widget);
			else
				gtk_widget_show(widget);
		}

		if (visible)
			gtk_widget_show(border);

		//g_debug("resize: %p %s: %d %d", this, name(), w, h);
		_dirty_size = true;

		#if GEOMETRY_OPTIMIZATION
		gApplication::setDirty();
		#else
		updateGeometry();
		#endif
	}

	if (pr)
		pr->performArrange();

	send_configure(this); // needed for Watcher and Form Resize events
}

void gControl::moveResize(int x, int y, int w, int h)
{
	if (pr)
		pr->disableArrangement();

	resize(w, h);
	move(x, y);

	if (pr)
		pr->enableArrangement();
}

void gControl::updateGeometry()
{
// 	if (_dirty_pos)
// 	{
// 		g_debug("move: %p -> %d %d", this, x(), y());
// 		_dirty_pos = false;
// 		GtkLayout *fx = GTK_LAYOUT(gtk_widget_get_parent(border));
// 		gtk_layout_move(fx, border, x(), y());
// 	}
//
// 	if (_dirty_size)
// 	{
// 		GtkAllocation a = { x(), y(), width(), height() };
// 		g_debug("resize: %p -> %d %d", this, width(), height());
// 		_dirty_size = false;
// 		//gtk_widget_set_size_request(border, width(), height());
// 		gtk_widget_size_allocate(border,
// 	}
	if (_dirty_pos || _dirty_size)
	{
		//g_debug("move-resize: %s: %d %d %d %d", this->name(), x(), y(), width(), height());
		if (_dirty_pos)
		{
			if (pr)
				pr->moveChild(this, x(), y());

			_dirty_pos = false;
		}
		if (_dirty_size && visible)
		{
			gtk_widget_set_size_request(border, width(), height());
			_dirty_size = false;
		}
	}
}

/*****************************************************************

APPEARANCE

******************************************************************/


void gControl::setExpand (bool vl)
{
	if (vl == expa)
		return;

	expa=vl;

	if (pr) pr->performArrange();
}


void gControl::setIgnore (bool vl)
{
	if (vl == igno)
		return;

	igno=vl;
	if (pr) pr->performArrange();
}

void gControl::setTooltip(char *vl)
{
	char *pango;
	
	if (_tooltip) g_free(_tooltip);
	_tooltip = NULL;
	if (vl) _tooltip = g_strdup(vl);
	
	if (_tooltip)
	{
		pango = gt_html_to_pango_string(_tooltip, -1, false);
		gtk_widget_set_tooltip_markup(border, pango);
		g_free(pango);
	}
	else
		gtk_widget_set_tooltip_markup(border, NULL);
}

gFont* gControl::font()
{
	if (_resolved_font)
	{
		//fprintf(stderr, "%s: font -> _resolved_font\n", name());
		return _resolved_font;
	}
	else if (pr)
	{
		//fprintf(stderr, "%s: font -> parent\n", name());
		return pr->font();
	}
	else
	{
		//fprintf(stderr, "%s: font -> desktop\n", name());
		return gDesktop::font();
	}
}

void gControl::actualFontTo(gFont *ft)
{
	font()->copyTo(ft);
}

void gControl::resolveFont()
{
	gFont *font;

	if (_font)
	{
		font = new gFont();
		font->mergeFrom(_font);
		if (pr)
			font->mergeFrom(pr->font());
		else
			font->mergeFrom(gDesktop::font());

		gFont::set(&_resolved_font, font);
	}
	else
		gFont::assign(&_resolved_font);
}

void gControl::setFont(gFont *ft)
{
	//fprintf(stderr, "setFont: %s: %s\n", name(), ft->toFullString());
	if (ft)
		gFont::assign(&_font, ft);
	else if (_font)
		gFont::assign(&_font);

	gFont::assign(&_resolved_font);

	updateFont();

	resize();

	//fprintf(stderr, "--> %s: _font = %s\n", name(), _font ? _font->toFullString() : NULL);
}

#ifdef GTK3

void gControl::updateFont()
{
	resolveFont();
	updateStyleSheet();
	updateSize();
}

#else

static void cb_update_font(GtkWidget *widget, gpointer data)
{
	PangoFontDescription *desc = (PangoFontDescription *)data;
	gtk_widget_modify_font(widget, desc);
}

void gControl::updateFont()
{
	resolveFont();
	gtk_widget_modify_font(widget, font()->desc());
	if (!isContainer() && GTK_IS_CONTAINER(widget))
		gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)cb_update_font, (gpointer)font()->desc());
	refresh();
	updateSize();
}

#endif

void gControl::updateSize()
{
}

int gControl::mouse()
{
	if (_proxy)
		return _proxy->mouse();
	else
		return _mouse;
}

gCursor* gControl::cursor()
{
	if (_proxy)
		return _proxy->cursor();

	if (!curs) return NULL;
	return new gCursor(curs);
}

void gControl::setCursor(gCursor *vl)
{
	if (_proxy)
	{
		_proxy->setCursor(vl);
		return;
	}

	if (curs) { delete curs; curs=NULL;}
	if (!vl)
	{
		setMouse(CURSOR_DEFAULT);
		return;
	}
	curs=new gCursor(vl);
	setMouse(CURSOR_CUSTOM);
}

void gControl::updateCursor(GdkCursor *cursor)
{
	if (GDK_IS_WINDOW(gtk_widget_get_window(border)) && _inside)
	{
		//fprintf(stderr, "updateCursor: %s %p\n", name(), cursor);
		if (!cursor && parent() && gtk_widget_get_window(parent()->border) == gtk_widget_get_window(border))
			parent()->updateCursor(parent()->getGdkCursor());
		else
			gdk_window_set_cursor(gtk_widget_get_window(border), cursor);
	}
}

GdkCursor *gControl::getGdkCursor()
{
	const char *name;
	GdkCursor *cr = NULL;
	int m = _mouse;

	if (gApplication::isBusy())
		m = GDK_WATCH;

	if (m == CURSOR_CUSTOM)
	{
		if (curs && curs->cur)
			return curs->cur;
	}

	if (m != CURSOR_DEFAULT)
	{
		switch(m)
		{
			case GDK_BLANK_CURSOR: name = "none"; break;
			case GDK_LEFT_PTR: name = "default"; break;
			case GDK_CROSSHAIR: name = "crosshair"; break;
			case GDK_WATCH: name = "wait"; break;
			case GDK_XTERM: name = "text"; break;
			case GDK_FLEUR: name = "move"; break;
			case GDK_SB_H_DOUBLE_ARROW: name = "ew-resize"; break;
			case GDK_SB_V_DOUBLE_ARROW: name = "ns-resize"; break;
			case GDK_TOP_SIDE: name = "n-resize"; break;
			case GDK_BOTTOM_SIDE: name = "s-resize"; break;
			case GDK_LEFT_SIDE: name = "w-resize"; break;
			case GDK_RIGHT_SIDE: name = "e-resize"; break;
			case GDK_TOP_LEFT_CORNER: name = "nw-resize"; break;
			case GDK_BOTTOM_RIGHT_CORNER: name = "se-resize"; break;
			case GDK_TOP_RIGHT_CORNER: name = "ne-resize"; break;
			case GDK_BOTTOM_LEFT_CORNER: name = "sw-resize"; break;
			case GDK_LAST_CURSOR+1: name = "nwse-resize"; break;
			case GDK_LAST_CURSOR+2: name = "nesw-resize"; break;
			case GDK_HAND2: name = "pointer"; break;
			default: name = "default";
		}
		
		cr = gdk_cursor_new_from_name(gdk_display_get_default(), name);
		if (!cr)
			cr = gdk_cursor_new_for_display(gdk_display_get_default(), (GdkCursorType)m);
		
		/*
		if (m < GDK_LAST_CURSOR)
		{
			cr = gdk_cursor_new_for_display(gdk_display_get_default(), (GdkCursorType)m);
		}
		else
		{
			if (m == (GDK_LAST_CURSOR+1)) //FDiag
			{
				pix = gdk_pixbuf_new_from_xpm_data(_cursor_fdiag);
				cr = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), pix, 8, 8);
				g_object_unref(pix);
			}
			else if (m == (GDK_LAST_CURSOR+2)) //BDiag
			{
				pix = gdk_pixbuf_new_from_xpm_data(_cursor_bdiag);
				cr = gdk_cursor_new_from_pixbuf(gdk_display_get_default(), pix, 8, 8);
				g_object_unref(pix);
			}
		}*/
	}

	return cr;
}

void gControl::setMouse(int m)
{
	if (_proxy)
	{
		_proxy->setMouse(m);
		return;
	}

	if (m == CURSOR_CUSTOM)
	{
		if (!curs || !curs->cur)
			m = CURSOR_DEFAULT;
	}

	_mouse = m;

	updateCursor(getGdkCursor());
}



/*****************************************************************

HANDLES

******************************************************************/

bool gControl::isWindow() const
{
	return g_typ == Type_gMainWindow;
}

gMainWindow* gControl::window()
{
	if (isWindow())
		return (gMainWindow *)this;

	if (!pr)
		return NULL;
	else
		return pr->window();
}

gMainWindow* gControl::topLevel()
{
	gControl *child = this;

	while (!child->isTopLevel())
		child = child->parent();

	return (gMainWindow *)child;
}

int gControl::handle()
{
	#ifdef GDK_WINDOWING_X11
	if (MAIN_display_x11)
	{
		GdkWindow *window = gtk_widget_get_window(border);
		return window ? GDK_WINDOW_XID(window) : 0;
	}
	else
		return 0;
	#else
	stub("no-X11/gControl::handle()");
	return 0;
	#endif
}

/*****************************************************************

MISC

******************************************************************/

void gControl::refresh()
{
	//refresh(0, 0, 0, 0);
	gtk_widget_queue_draw(border);
	if (frame != border && GTK_IS_WIDGET(frame))
		gtk_widget_queue_draw(frame);
	if (widget != frame  && GTK_IS_WIDGET(widget))
		gtk_widget_queue_draw(widget);

	afterRefresh();
}

void gControl::refresh(int x, int y, int w, int h)
{
	GdkRectangle r;
	GtkAllocation a;

	gtk_widget_get_allocation(border, &a);

	if (x < 0 || y < 0 || w <= 0 || h <= 0)
	{
		x = y = 0;
		w = width();
		h = height();
	}
	r.x = a.x + x;
	r.y = a.y + y;
	r.width = w;
	r.height = h;

	gdk_window_invalidate_rect(gtk_widget_get_window(border), &r, TRUE);

	afterRefresh();
}

void gControl::afterRefresh()
{
}

bool gControl::design()
{
	return dsg;
}

void gControl::setDesign(bool vl)
{
	dsg=vl;
}

void gControl::setFocus()
{
	if (_proxy)
	{
		_proxy->setFocus();
		return;
	}

	gMainWindow *win = window();

	if (!win)
		return;

	if (win->isVisible())
	{
		//if (isVisible() && bufW > 0 && bufH > 0)
		//fprintf(stderr, "setFocus now %s\n", name());
		gtk_widget_grab_focus(widget);
	}
	else
	{
		//fprintf(stderr, "setFocus later %s\n", name());
		win->focus = this;
	}
}

bool gControl::hasFocus() const
{
	if (_proxy)
		return _proxy->hasFocus();
	else
#if GTK_CHECK_VERSION(2, 18, 0)
		return (border && gtk_widget_has_focus(border)) || (widget && gtk_widget_has_focus(widget)) || gApplication::activeControl() == this;
#else
		return (border && GTK_WIDGET_HAS_FOCUS(border)) || (widget && GTK_WIDGET_HAS_FOCUS(widget)) || gApplication::activeControl() == this;
#endif
}

#if GTK_CHECK_VERSION(3, 2, 0)
bool gControl::hasVisibleFocus() const
{
	if (_proxy)
		return _proxy->hasVisibleFocus();
	else
		return (border && gtk_widget_has_visible_focus(border)) || (widget && gtk_widget_has_visible_focus(widget));
}
#endif

gControl* gControl::next()
{
	int index;

	if (!pr)
		return NULL;

	index = pr->childIndex(this);
	if (index < 0 || index >= pr->childCount())
		return NULL;
	else
		return pr->child(index + 1);
}

gControl* gControl::previous()
{
	int index;

	if (!pr)
		return NULL;

	index = pr->childIndex(this);
	if (index <= 0)
		return NULL;
	else
		return pr->child(index - 1);
}


void gControl::lower()
{
	gpointer *p;
	GList *chd;
	GtkWidget *child;
	gControl *br;
	int x,y;
	GtkContainer *parent;

	if (!pr) return;

	/*if (gtk_widget_get_has_window(border))
	{
		gdk_window_lower(gtk_widget_get_window(border));
		if (gtk_widget_get_window(widget))
			gdk_window_lower(gtk_widget_get_window(widget));
	}
	else*/
	{
		//fprintf(stderr, "gb.gtk: warning: gControl::lower(): no window\n");

		if (!(chd=gtk_container_get_children(GTK_CONTAINER(pr->getContainer())))) return;

		chd = g_list_first(chd);

		while(chd)
		{
			child = (GtkWidget*)chd->data;

			br = gt_get_control(child);

			if (br && br != this)
			{
				x = br->x();
				y = br->y();
				parent = GTK_CONTAINER(gtk_widget_get_parent(br->border));
				g_object_ref(G_OBJECT(br->border));
				gtk_container_remove(parent, br->border);
				gtk_container_add(parent, br->border);

				if (GTK_IS_LAYOUT(parent))
					gtk_layout_move(GTK_LAYOUT(parent), br->border, x, y);
				else
					gtk_fixed_move(GTK_FIXED(parent), br->border, x, y);

				g_object_unref(G_OBJECT(br->border));
			}

			chd = g_list_next(chd);
		}
	}

	g_ptr_array_remove(pr->_children, this);

	g_ptr_array_add(pr->_children, NULL);
	p = pr->_children->pdata;
	g_memmove(&p[1], &p[0], (pr->_children->len - 1) * sizeof(gpointer));
	p[0] = this;

	//pr->ch_list = g_list_remove(pr->ch_list, this);
	//pr->ch_list = g_list_prepend(pr->ch_list, this);
	pr->updateFocusChain();
	pr->performArrange();
}

void gControl::raise()
{
	int x, y;
	GtkContainer *parent;

	if (!pr) return;

	x = left();
	y = top();
	parent = GTK_CONTAINER(gtk_widget_get_parent(border));
	g_object_ref(G_OBJECT(border));
	gtk_container_remove(parent, border);
	gtk_container_add(parent, border);

	//pr->moveChild(this, x, y);
	if (GTK_IS_LAYOUT(parent))
		gtk_layout_move(GTK_LAYOUT(parent), border, x, y);
	else
		gtk_fixed_move(GTK_FIXED(parent), border, x, y);

	g_object_unref(G_OBJECT(border));

	g_ptr_array_remove(pr->_children, this);
	g_ptr_array_add(pr->_children, this);

	pr->updateFocusChain();
	pr->performArrange();
}

void gControl::setNext(gControl *ctrl)
{
	#ifdef GAMBAS_DIRECTFB
	stub("DIRECTFB/gControl::setNext()");
	#else
	Window stack[2];
	GPtrArray *ch;
	uint i;

	if (!ctrl)
	{
		raise();
		return;
	}

	if (ctrl == this || isTopLevel() || ctrl->parent() != parent())
		return;

	if (gtk_widget_get_has_window(ctrl->border) && gtk_widget_get_has_window(border))
	{
		stack[0] = GDK_WINDOW_XID(gtk_widget_get_window(ctrl->border));
		stack[1] = GDK_WINDOW_XID(gtk_widget_get_window(border));

		XRestackWindows(GDK_WINDOW_XDISPLAY(gtk_widget_get_window(border)), stack, 2 );
	}

	ch = pr->_children;
	g_ptr_array_remove(ch, this);
	g_ptr_array_add(ch, NULL);

	for (i = 0; i < ch->len; i++)
	{
		if (g_ptr_array_index(ch, i) == ctrl)
		{
			g_memmove(&ch->pdata[i + 1], &ch->pdata[i], (ch->len - i - 1) * sizeof(gpointer));
			ch->pdata[i] = this;
			break;
		}
	}

	pr->updateFocusChain();
	pr->performArrange();
	#endif
}

void gControl::setPrevious(gControl *ctrl)
{
	if (!ctrl)
		lower();
	else
		setNext(ctrl->next());
}

/*********************************************************************

Drag & Drop

**********************************************************************/

void gControl::setAcceptDrops(bool vl)
{
	//GtkWidget *w;
	//GtkTargetEntry entry[7];

	/* BM: ??
	if (!pr) w=frame;
	else w=widget;
	*/

	if (vl == _accept_drops)
		return;

	_accept_drops = vl;

	if (vl)
	{
		gtk_drag_dest_set(border, (GtkDestDefaults)0, NULL, 0, (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));
		//if (widget != border)
		//	gtk_drag_dest_set(widget, (GtkDestDefaults)0, NULL, 0, (GdkDragAction)(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));
	}
	else
	{
		gtk_drag_dest_unset(border);
		//if (widget != border)
		//	gtk_drag_dest_unset(widget);
	}
}

/*********************************************************************

Internal

**********************************************************************/

void gControl::connectParent()
{
	if (pr)
	{
		//gtk_widget_set_redraw_on_allocate(border, false);

		pr->insert(this, true);
	}

	// BM: Widget has been created, so we can set its cursor if application is busy
	if (gApplication::isBusy() && mustUpdateCursor())
		setMouse(mouse());
}

GList* gControl::controlList()
{
	return controls;
}

gColor gControl::getFrameColor()
{
	return gDesktop::lightfgColor();
}

#ifdef GTK3
void gControl::drawBorder(cairo_t *cr)
{
	gt_draw_border(cr, gtk_widget_get_style_context(widget), GTK_STATE_FLAG_NORMAL, getFrameBorder(), getFrameColor(), 0, 0, width(), height(), use_base);
}
#else
void gControl::drawBorder(GdkEventExpose *e)
{
	GdkWindow *win;
	GtkShadowType shadow;
	gint x, y, w, h;
	cairo_t *cr;
	GtkWidget *wid;
	GtkAllocation a;

	if (getFrameBorder() == BORDER_NONE)
		return;

	x = 0;
	y = 0;
	w = width();
	h = height();

	if (frame)
		wid = frame;
	else
		wid = widget;

	if (GTK_IS_LAYOUT(wid))
		win = gtk_layout_get_bin_window(GTK_LAYOUT(wid));
	else
		win = gtk_widget_get_window(wid);

	gtk_widget_get_allocation(wid, &a);
	x = a.x;
	y = a.y;

	if (w < 1 || h < 1)
		return;

	switch (getFrameBorder())
	{
		case BORDER_PLAIN:

			cr = gdk_cairo_create(win);
			gt_cairo_draw_rect(cr, x, y, w, h, getFrameColor());
			cairo_destroy(cr);
			return;

		case BORDER_SUNKEN: shadow = GTK_SHADOW_IN; break;
		case BORDER_RAISED: shadow = GTK_SHADOW_OUT; break;
		case BORDER_ETCHED: shadow = GTK_SHADOW_ETCHED_IN; break;

		default:
			return;
	}

	GdkRectangle clip;
	gdk_region_get_clipbox(e->region, &clip);
	GtkStyle *st = gtk_widget_get_style(widget);
	if (use_base)
		gtk_paint_box(st, win, GTK_STATE_NORMAL, shadow, &clip, widget, "entry", x, y, w, h);
	else
		gtk_paint_shadow(st, win, GTK_STATE_NORMAL, shadow, &clip, widget, NULL, x, y, w, h);
}
#endif

/*static void cb_size_allocate(GtkWidget *wid, GtkAllocation *a, gContainer *container)
{
	if (!container->isTopLevel())
		container->performArrange();
}*/


/*
	The different cases managed by gControl::realize()

	border     frame      widget
		0          0          W
		0          F          W
		B          0          W
		B          F          W
*/

static void add_container(GtkWidget *parent, GtkWidget *child)
{
	GtkWidget *ch;

	for(;;)
	{
		if (!GTK_IS_BIN(parent))
			break;

		ch = gtk_bin_get_child(GTK_BIN(parent));
		if (!ch)
			break;

		parent = ch;
	}

	gtk_container_add(GTK_CONTAINER(parent), child);
}

void gControl::registerControl()
{
	gt_register_control(border, this);
}

#ifdef GTK3
/*static gboolean cb_clip_children(GtkWidget *wid, GdkEventExpose *e, gContainer *d)
{
	cairo_region_t *me;
	GtkAllocation a;

	gtk_widget_get_allocation(wid, &a);
	me = cairo_region_create_rectangle((cairo_rectangle_int_t *)&a);

	cairo_region_intersect(e->region, me);

	cairo_region_destroy(me);

	if (cairo_region_is_empty(e->region))
		return TRUE;

	return FALSE;
}*/
#else
static gboolean cb_clip_children(GtkWidget *wid, GdkEventExpose *e, gContainer *d)
{
	GdkRegion *me;
	GtkAllocation a;

	gtk_widget_get_allocation(wid, &a);
	me = gdk_region_rectangle((GdkRectangle *)&a);

	gdk_region_intersect(e->region, me);

	gdk_region_destroy(me);

	if (gdk_region_empty(e->region))
		return TRUE;

	return FALSE;
}
#endif

#if 0
static gboolean cb_clip_by_parent(GtkWidget *wid, GdkEventExpose *e, gControl *d)
{
	GdkRegion *preg;
	GdkRectangle prect = { 0, 0, d->parent()->width() - d->x(), d->parent()->height() - d->y() };

	fprintf(stderr, "area = %d %d %d %d  prect = %d %d %d %d\n",
					e->area.x, e->area.y, e->area.width, e->area.height,
					prect.x, prect.y, prect.width, prect.height);

	preg = gdk_region_rectangle(&prect);

	gdk_region_intersect(e->region, preg);

	gdk_region_destroy(preg);

	if (gdk_region_empty(e->region))
		return TRUE;

	gdk_region_get_clipbox(e->region, &prect);
	e->area = prect;
	fprintf(stderr, "--> %d %d %d %d\n", prect.x, prect.y, prect.width, prect.height);

	return FALSE;
}
#endif

#ifdef GTK3

//fprintf(stderr, "get_preferred_width [%p %s] %p\n", klass, G_OBJECT_TYPE_NAME(widget), klass->_gtk_reserved2);
//fprintf(stderr, "get_preferred_height [%p %s] %p\n", klass, G_OBJECT_TYPE_NAME(widget), klass->_gtk_reserved3);

//#define must_patch(_widget) (gt_get_control(_widget) != NULL)

static bool must_patch(GtkWidget *widget)
{
	GtkWidget *parent;
	gControl *parent_control;

	if (GTK_IS_ENTRY(widget))
		return true;

	if (gt_get_control(widget))
		return true;

	parent = gtk_widget_get_parent(widget);
	if (!parent)
		return false;
	
	if (GTK_IS_SCROLLED_WINDOW(parent))
	{
		parent = gtk_widget_get_parent(parent);
		if (!parent)
			return false;
	}

	parent_control = gt_get_control(parent);
	if (!parent_control)
		return false;

	return (parent_control->widget == widget || (GtkWidget *)parent_control->_scroll == widget);
}

//fprintf(stderr, #type "_get_preferred_height: %d %d\n", minimum_size ? *minimum_size : -1, natural_size ? *natural_size : -1);
//fprintf(stderr, #type "_get_preferred_width: %d %d\n", minimum_size ? *minimum_size : -1, natural_size ? *natural_size : -1);

#define OLD_FUNC ((PATCH_FUNCS *)(klass->_gtk_reserved6))

#define PATCH_DECLARE_COMMON(_type, _name) \
static void _name##get_preferred_width(GtkWidget *widget, gint *minimum_size, gint *natural_size) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass*)g_type_class_peek(_type); \
	(*OLD_FUNC->get_preferred_width)(widget, minimum_size, natural_size); \
	if (minimum_size && must_patch(widget)) \
		*minimum_size = 0; \
} \
static void _name##get_preferred_height(GtkWidget *widget, gint *minimum_size, gint *natural_size) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(_type); \
	(*OLD_FUNC->get_preferred_height)(widget, minimum_size, natural_size); \
	if (minimum_size && must_patch(widget)) \
		*minimum_size = 0; \
} \
static void _name##get_preferred_height_for_width(GtkWidget *widget, gint width, gint *minimum_size, gint *natural_size) \
{ \
	if (minimum_size && must_patch(widget)) \
	{ \
		*minimum_size = 0; \
		*natural_size = 0; \
		return; \
	} \
	GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(_type); \
	(*OLD_FUNC->get_preferred_height_for_width)(widget, width, minimum_size, natural_size); \
} \
static void _name##get_preferred_width_for_height(GtkWidget *widget, gint height, gint *minimum_size, gint *natural_size) \
{ \
	if (minimum_size && must_patch(widget)) \
	{ \
		*minimum_size = 0; \
		*natural_size = 0; \
		return; \
	} \
	GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(_type); \
	(*OLD_FUNC->get_preferred_height_for_width)(widget, height, minimum_size, natural_size); \
}

#if GTK_CHECK_VERSION(3, 14, 0)

#define PATCH_DECLARE_SIZE(_type, _name) \
static void _name##size_allocate(GtkWidget *widget, GtkAllocation *allocation) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(_type); \
	(*OLD_FUNC->size_allocate)(widget, allocation); \
	gtk_widget_set_clip(widget, allocation); \
}

#elif GTK_CHECK_VERSION(3, 10, 0)

#define PATCH_DECLARE_SIZE(_type, _name) \
static void _name##size_allocate(GtkWidget *widget, GtkAllocation *allocation) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(_type); \
	(*OLD_FUNC->size_allocate)(widget, allocation); \
}

#else

#define PATCH_DECLARE_SIZE(_type, _name) 

#endif

#define PATCH_DECLARE(type) PATCH_DECLARE_COMMON(type, type##_) PATCH_DECLARE_SIZE(type, type##_)

#define PATCH_DECLARE_BASELINE(type) \
static void type##_get_preferred_height_and_baseline_for_width(GtkWidget *widget, gint width, gint *minimum, gint *natural, gint *minimum_baseline, gint *natural_baseline) \
{ \
	if (minimum && minimum_baseline && must_patch(widget)) \
	{ \
		GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(type); \
		if (OLD_FUNC->get_preferred_height_and_baseline_for_width) \
			(*OLD_FUNC->get_preferred_height_and_baseline_for_width)(widget, width, minimum, natural, minimum_baseline, natural_baseline); \
		else \
		{ \
			*minimum_baseline = 0; \
			*natural_baseline = 0; \
		} \
		*minimum = 0; \
		*natural = 0; \
		return; \
	} \
	GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(type); \
	(*OLD_FUNC->get_preferred_height_and_baseline_for_width)(widget, width, minimum, natural, minimum_baseline, natural_baseline); \
}

//fprintf(stderr, "patching [%p %s] (%p %p)\n", klass, G_OBJECT_TYPE_NAME(widget), klass->get_preferred_width, klass->get_preferred_height);
//		fprintf(stderr, "PATCH_CLASS: %s\n", G_OBJECT_TYPE_NAME(widget));

#if GTK_CHECK_VERSION(3,10,0)

#define PATCH_CLASS(widget, type) \
if (G_OBJECT_TYPE(widget) == type) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)GTK_WIDGET_GET_CLASS(widget); \
	if (klass->get_preferred_width != type##_get_preferred_width) \
	{ \
		PATCH_FUNCS *funcs = g_new0(PATCH_FUNCS, 1); \
		funcs->get_preferred_width = klass->get_preferred_width; \
		funcs->get_preferred_height = klass->get_preferred_height; \
		funcs->get_preferred_height_for_width = klass->get_preferred_height_for_width; \
		funcs->get_preferred_width_for_height = klass->get_preferred_width_for_height; \
		funcs->size_allocate = klass->size_allocate; \
		klass->_gtk_reserved6 = (void(*)())funcs; \
		klass->get_preferred_width = type##_get_preferred_width; \
		klass->get_preferred_height = type##_get_preferred_height; \
		klass->get_preferred_height_for_width = type##_get_preferred_height_for_width; \
		klass->get_preferred_width_for_height = type##_get_preferred_width_for_height; \
		klass->size_allocate = type##_size_allocate; \
	} \
}

#define PATCH_CLASS_BASELINE(widget, type) \
if (G_OBJECT_TYPE(widget) == type) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)GTK_WIDGET_GET_CLASS(widget); \
	if (klass->get_preferred_width != type##_get_preferred_width) \
	{ \
		PATCH_FUNCS *funcs = g_new0(PATCH_FUNCS, 1); \
		funcs->get_preferred_width = klass->get_preferred_width; \
		funcs->get_preferred_height = klass->get_preferred_height; \
		funcs->get_preferred_height_for_width = klass->get_preferred_height_for_width; \
		funcs->get_preferred_width_for_height = klass->get_preferred_width_for_height; \
		funcs->size_allocate = klass->size_allocate; \
		funcs->get_preferred_height_and_baseline_for_width = klass->get_preferred_height_and_baseline_for_width; \
		klass->_gtk_reserved6 = (void(*)())funcs; \
		klass->get_preferred_width = type##_get_preferred_width; \
		klass->get_preferred_height = type##_get_preferred_height; \
		klass->get_preferred_height_for_width = type##_get_preferred_height_for_width; \
		klass->get_preferred_width_for_height = type##_get_preferred_width_for_height; \
		klass->size_allocate = type##_size_allocate; \
		klass->get_preferred_height_and_baseline_for_width = type##_get_preferred_height_and_baseline_for_width; \
	} \
}

#else

#define PATCH_CLASS(widget, type) \
if (G_OBJECT_TYPE(widget) == type) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)GTK_WIDGET_GET_CLASS(widget); \
	if (klass->get_preferred_width != type##_get_preferred_width) \
	{ \
		PATCH_FUNCS *funcs = g_new0(PATCH_FUNCS, 1); \
		funcs->get_preferred_width = klass->get_preferred_width; \
		funcs->get_preferred_height = klass->get_preferred_height; \
		funcs->get_preferred_height_for_width = klass->get_preferred_height_for_width; \
		funcs->get_preferred_width_for_height = klass->get_preferred_width_for_height; \
		klass->_gtk_reserved6 = (void(*)())funcs; \
		klass->get_preferred_width = type##_get_preferred_width; \
		klass->get_preferred_height = type##_get_preferred_height; \
		klass->get_preferred_height_for_width = type##_get_preferred_height_for_width; \
		klass->get_preferred_width_for_height = type##_get_preferred_width_for_height; \
	} \
}

#define PATCH_CLASS_BASELINE PATCH_CLASS

#endif

PATCH_DECLARE(GTK_TYPE_WINDOW)
PATCH_DECLARE(GTK_TYPE_ENTRY)
PATCH_DECLARE(GTK_TYPE_COMBO_BOX)
PATCH_DECLARE(GTK_TYPE_SPIN_BUTTON)
PATCH_DECLARE(GTK_TYPE_BUTTON)
PATCH_DECLARE(GTK_TYPE_FIXED)
PATCH_DECLARE(GTK_TYPE_EVENT_BOX)
//PATCH_DECLARE(GTK_TYPE_ALIGNMENT)
PATCH_DECLARE(GTK_TYPE_BOX)
PATCH_DECLARE(GTK_TYPE_TOGGLE_BUTTON)
PATCH_DECLARE(GTK_TYPE_SCROLLED_WINDOW)
PATCH_DECLARE(GTK_TYPE_CHECK_BUTTON)
PATCH_DECLARE(GTK_TYPE_RADIO_BUTTON)
PATCH_DECLARE(GTK_TYPE_NOTEBOOK)
PATCH_DECLARE(GTK_TYPE_SOCKET)
PATCH_DECLARE(GTK_TYPE_TEXT_VIEW)
PATCH_DECLARE(GTK_TYPE_SCROLLBAR)
PATCH_DECLARE(GTK_TYPE_SCALE)

#if GTK_CHECK_VERSION(3,10,0)
PATCH_DECLARE_BASELINE(GTK_TYPE_ENTRY)
PATCH_DECLARE_BASELINE(GTK_TYPE_COMBO_BOX)
PATCH_DECLARE_BASELINE(GTK_TYPE_SPIN_BUTTON)
PATCH_DECLARE_BASELINE(GTK_TYPE_BUTTON)
#endif

/*int gt_get_preferred_width(GtkWidget *widget)
{
	int m, n;
	GtkWidgetClass *klass = (GtkWidgetClass*)g_type_class_peek(G_OBJECT_TYPE(widget));
	if (klass->_gtk_reserved6)
		(*(void (*)(GtkWidget *, gint *, gint *))klass->_gtk_reserved6)(widget, &m, &n);
	return m;
}*/

#endif

void gControl::realize(bool make_frame)
{
	if (!_scroll)
	{
		if (!make_frame)
		{
			frame = widget;
		}
		else if (!frame)
		{
#if GTK3
			border = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_widget_set_hexpand(widget, TRUE);
#else
			border = gtk_alignment_new(0, 0, 1, 1);
#endif
			gtk_widget_set_redraw_on_allocate(frame, TRUE);
		}

		if (!border)
			border = widget;

		//printf("border = %p / frame = %p / widget =%p\n", border, frame, widget);

		if (border != frame)
		{
			//printf("frame -> border\n");
			add_container(border, frame);
		}
		if (frame != widget && border != widget)
		{
			//printf("widget -> frame\n");
			add_container(frame, widget);
		}

		if (!make_frame)
			frame = 0;
	}

	//fprintf(stderr, "realize: %p %p\n", border, widget);
	
#ifdef GTK3

	PATCH_CLASS(border, GTK_TYPE_WINDOW)
	else PATCH_CLASS_BASELINE(border, GTK_TYPE_ENTRY)
	else PATCH_CLASS_BASELINE(border, GTK_TYPE_SPIN_BUTTON)
	else PATCH_CLASS_BASELINE(border, GTK_TYPE_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_FIXED)
	else PATCH_CLASS(border, GTK_TYPE_EVENT_BOX)
	//else PATCH_CLASS(border, GTK_TYPE_ALIGNMENT)
	else PATCH_CLASS(border, GTK_TYPE_BOX)
	else PATCH_CLASS(border, GTK_TYPE_TOGGLE_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_SCROLLED_WINDOW)
	else PATCH_CLASS(border, GTK_TYPE_CHECK_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_RADIO_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_NOTEBOOK)
	else PATCH_CLASS(border, GTK_TYPE_SOCKET)
	else PATCH_CLASS(border, GTK_TYPE_TEXT_VIEW)
	else PATCH_CLASS(border, GTK_TYPE_SCROLLBAR)
	else PATCH_CLASS(border, GTK_TYPE_SCALE)
	else 
	{
		fprintf(stderr, "gb.gtk3: warning: class %s was not patched\n", G_OBJECT_TYPE_NAME(border));
	}

	PATCH_CLASS_BASELINE(widget, GTK_TYPE_COMBO_BOX)
	else PATCH_CLASS(widget, GTK_TYPE_TEXT_VIEW)

#endif

	connectParent();
	initSignals();

//#ifndef GTK3
	if (!_no_background && !gtk_widget_get_has_window(border))
		//g_signal_connect(G_OBJECT(border), "expose-event", G_CALLBACK(cb_draw_background), (gpointer)this);
		ON_DRAW_BEFORE(border, this, cb_background_expose, cb_background_draw);

	if (frame)
		ON_DRAW_BEFORE(frame, this, cb_frame_expose, cb_frame_draw);
//#endif

	/*else if (!isTopLevel())
	{
		fprintf(stderr, "clip by parent\n");
		g_signal_connect(G_OBJECT(border), "expose-event", G_CALLBACK(cb_clip_by_parent), (gpointer)this);
	}*/

#ifndef GTK3
	if (isContainer() && !gtk_widget_get_has_window(widget))
		g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(cb_clip_children), (gpointer)this);
#endif

	//if (isContainer() && widget != border)
	//	g_signal_connect(G_OBJECT(widget), "size-allocate", G_CALLBACK(cb_size_allocate), (gpointer)this);

	gtk_widget_add_events(widget, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK
		| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	if (widget != border && (GTK_IS_WINDOW(border) || (GTK_IS_EVENT_BOX(border) && !gtk_event_box_get_visible_window(GTK_EVENT_BOX(border)))))
	{
		gtk_widget_add_events(border, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK
			| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	}

	registerControl();
	updateFont();
}

void gControl::realizeScrolledWindow(GtkWidget *wid, bool doNotRealize)
{
	_scroll = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));

#ifdef GTK3
	PATCH_CLASS(_scroll, GTK_TYPE_SCROLLED_WINDOW)
	PATCH_CLASS(wid, GTK_TYPE_TEXT_VIEW)
#endif

#ifdef GTK3
		border = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_widget_set_hexpand(wid, TRUE);
#else
		border = gtk_alignment_new(0, 0, 1, 1);
#endif
	gtk_widget_set_redraw_on_allocate(border, TRUE);
	widget = wid;
	frame = border;
	_no_auto_grab = true;

	//gtk_container_add(GTK_CONTAINER(border), GTK_WIDGET(_scroll));
	gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(_scroll, GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(border), GTK_WIDGET(_scroll));
	gtk_container_add(GTK_CONTAINER(_scroll), widget);

	if (!doNotRealize)
		realize(false);
	else
		registerControl();

	updateFont();
}

void gControl::updateBorder()
{
	int pad;

	if (!frame)
		return;

#if GTK3
	if (!GTK_IS_BOX(frame))
#else
	if (!GTK_IS_ALIGNMENT(frame))
#endif
	{
		refresh();
		return;
	}

	switch (frame_border)
	{
		case BORDER_NONE: pad = 0; break;
		case BORDER_PLAIN: pad = 1; break;
		default: pad = gApplication::getFrameWidth(); break;
	}

	if ((int)frame_padding > pad)
		pad = frame_padding;

#if GTK3
	g_object_set(widget, "margin", pad, NULL);
#else
	gtk_alignment_set_padding(GTK_ALIGNMENT(frame), pad, pad, pad, pad);
	refresh();
#endif
	//gtk_widget_queue_draw(frame);
}

int gControl::getFrameWidth()
{
	guint p;

	if (frame)
	{
#if GTK3
		if (GTK_IS_BOX(frame))
		{
			g_object_get(widget, "margin", &p, NULL);
			return p;
		}
#else
		if (GTK_IS_ALIGNMENT(frame))
		{
			gtk_alignment_get_padding(GTK_ALIGNMENT(frame), &p, NULL, NULL, NULL);
			return p;
		}
#endif
	}

	/*if (_scroll)
	{
		if (gtk_scrolled_window_get_shadow_type(_scroll) == GTK_SHADOW_NONE)
			return 0;
		else
			return gApplication::getFrameWidth();
	}*/

	switch (frame_border)
	{
		case BORDER_NONE: p = 0; break;
		case BORDER_PLAIN: p = 1; break;
		default: p = gApplication::getFrameWidth(); break;
	}
	return p;
}

void gControl::setFrameBorder(int border)
{
	if (border < BORDER_NONE || border > BORDER_ETCHED)
		return;

	frame_border = border;
	updateBorder();
}

bool gControl::hasBorder() const
{
	return getFrameBorder() != BORDER_NONE;
}

void gControl::setBorder(bool vl)
{
	setFrameBorder(vl ? BORDER_SUNKEN : BORDER_NONE);
	_has_border = vl;
}


void gControl::setFramePadding(int padding)
{
	if (padding < 0)
		padding = 0;
	frame_padding = padding;
	updateBorder();
}


void gControl::setName(char *name)
{
	if (_name) g_free(_name);
	_name = NULL;
	if (name) _name = g_strdup(name);
}

#ifdef GTK3

GtkWidget *gControl::getStyleSheetWidget()
{
	return border;
}

void gControl::updateStyleSheet()
{
	static int count = 0;
	
	GtkWidget *wid;
	GtkStyleContext *context;
	char *css = NULL;
	const char *name;
	char buffer[128];
	int s;
	
	wid = getStyleSheetWidget();
	context = gtk_widget_get_style_context(wid);
	
	if (_bg == COLOR_DEFAULT && _fg == COLOR_DEFAULT && !_font)
	{
		if (_css)
			gtk_style_context_remove_provider(context, _css);
	}
	else
	{
		if (!_css)
		{
			count++;
			sprintf(buffer, "g%d", count);
			gtk_widget_set_name(wid, buffer);
			
			_css = GTK_STYLE_PROVIDER(gtk_css_provider_new());
		}
		else
			gtk_style_context_remove_provider(context, _css);
		
		name = gtk_widget_get_name(wid);
		sprintf(buffer, "#%s {\ntransition:none;\n", name);
		g_stradd(&css, buffer);
		
		if (_bg != COLOR_DEFAULT)
		{
			g_stradd(&css, "background-color:");
			gt_to_css_color(buffer, _bg);
			g_stradd(&css, buffer);
			g_stradd(&css, ";\nbackground-image:none;\n");
		}
		
		if (_fg != COLOR_DEFAULT)
		{
			g_stradd(&css, "color:");
			gt_to_css_color(buffer, _fg);
			g_stradd(&css, buffer);
			g_stradd(&css, ";\n");
		}
		
		if (_font)
		{
			if (_font->_name_set)
			{
				g_stradd(&css, "font-family:\"");
				g_stradd(&css, _font->name());
				g_stradd(&css, "\";\n");
			}
			
			if (_font->_size_set)
			{
				g_stradd(&css, "font-size:");
				s = (int)(_font->size() * 10 + 0.5);
				sprintf(buffer, "%dpt;\n", s / 10); //, s % 10);
				g_stradd(&css, buffer);
			}

			if (_font->_bold_set)
			{
				g_stradd(&css, "font-weight:");
				g_stradd(&css, _font->bold() ? "bold" : "normal");
				g_stradd(&css, ";\n");
			}
			
			if (_font->_italic_set)
			{
				g_stradd(&css, "font-style:");
				g_stradd(&css, _font->italic() ? "italic" : "normal");
				g_stradd(&css, ";\n");
			}

			if (_font->_underline_set || _font->_strikeout_set)
			{
				g_stradd(&css, "text-decoration-line:");
				if (_font->strikeout())
					g_stradd(&css, "line-through");
				else if (_font->underline())
					g_stradd(&css, "underline");
				else
					g_stradd(&css, "none");
				g_stradd(&css, ";\n");
			}
		}
		
		g_stradd(&css, "}\n");
		
		//fprintf(stderr, "---- %s\n%s", _name, css);
		
		gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(_css), css, -1, NULL);
		gtk_style_context_add_provider(context, _css, GTK_STYLE_PROVIDER_PRIORITY_USER + 10);
	}
}

gColor gControl::realBackground(bool no_default)
{
	if (_bg != COLOR_DEFAULT)
		return _bg;
	else if (pr)
		return pr->realBackground(no_default);
	else
		return no_default ? gDesktop::bgColor() : COLOR_DEFAULT;
}

gColor gControl::background()
{
	return _bg;
}

void gControl::setRealBackground(gColor color)
{
}

void gControl::setBackground(gColor color)
{
	_bg = color;
	updateStyleSheet();
	//gt_widget_set_color(border, FALSE, _bg, _bg_name, &_bg_default);
	updateColor();
}

gColor gControl::realForeground(bool no_default)
{
	if (_fg != COLOR_DEFAULT)
		return _fg;
	else if (pr)
		return pr->realForeground(no_default);
	else
		return no_default ? gDesktop::fgColor() : COLOR_DEFAULT;
}

gColor gControl::foreground()
{
	return _fg;
}

void gControl::setRealForeground(gColor color)
{
}

void gControl::setForeground(gColor color)
{
	_fg = color;
	updateStyleSheet();
	//gt_widget_set_color(border, TRUE, _fg, _fg_name, &_fg_default);
	updateColor();
}

#else

gColor gControl::realBackground(bool no_default)
{
	if (_bg_set)
		return use_base ? get_gdk_base_color(widget, isEnabled()) : get_gdk_bg_color(widget, isEnabled());
	else if (pr)
		return pr->realBackground(no_default);
	else
		return no_default ? gDesktop::bgColor() : COLOR_DEFAULT;
}

gColor gControl::background()
{
	return _bg;
}

static void set_background(GtkWidget *widget, gColor color, bool use_base)
{
	if (use_base)
		set_gdk_base_color(widget, color);
	else
		set_gdk_bg_color(widget, color);
}

void gControl::setRealBackground(gColor color)
{
	set_background(border, color, use_base);
	if (border != frame && GTK_IS_WIDGET(frame))
		set_background(frame, color, use_base);
	if (frame != widget)
		set_background(widget, color, use_base);
}

void gControl::setBackground(gColor color)
{
	_bg = color;
	_bg_set = color != COLOR_DEFAULT;

	if (!_bg_set)
	{
		if (pr && !use_base)
			color = pr->realBackground();
	}

	setRealBackground(color);
}

gColor gControl::realForeground(bool no_default)
{
	if (_fg_set)
		return use_base ? get_gdk_text_color(widget, isEnabled()) : get_gdk_fg_color(widget, isEnabled());
	else if (pr)
		return pr->realForeground(no_default);
	else
		return no_default ? gDesktop::fgColor() : COLOR_DEFAULT;
}

gColor gControl::foreground()
{
	return _fg;
}

static void set_foreground(GtkWidget *widget, gColor color, bool use_base)
{
	if (use_base)
		set_gdk_text_color(widget, color);
	else
		set_gdk_fg_color(widget, color);
}

void gControl::setRealForeground(gColor color)
{
	set_foreground(widget, color, use_base);
}

void gControl::setForeground(gColor color)
{
	_fg = color;
	_fg_set = color != COLOR_DEFAULT;

	if (!_fg_set)
	{
		if (pr)
			color = pr->realForeground();
	}

	setRealForeground(color);
}

#endif

void gControl::emit(void *signal)
{
	if (!signal || locked())
		return;
	(*((void (*)(gControl *))signal))(this);
}

void gControl::emit(void *signal, intptr_t arg)
{
	if (!signal || locked())
		return;
	(*((void (*)(gControl *, intptr_t))signal))(this, arg);
}

void gControl::reparent(gContainer *newpr, int x, int y)
{
	gContainer *oldpr;
	bool was_visible = isVisible();

	// newpr can be equal to pr: for example, to move a control for one
	// tab to another tab of the same TabStrip!

	if (!newpr || !newpr->getContainer())
		return;

	if (pr == newpr && pr->getContainer() == newpr->getContainer())
		return;

	if (was_visible) hide();
	//gtk_widget_unrealize(border);

	oldpr = pr;
	pr = newpr;

	if (oldpr == newpr)
	{
		gt_widget_reparent(border, newpr->getContainer());
		oldpr->performArrange();
	}
	else
	{
		if (oldpr)
		{
			gt_widget_reparent(border, newpr->getContainer());
			oldpr->remove(this);
			oldpr->performArrange();
		}

		newpr->insert(this);
	}

	//gtk_widget_realize(border);
	move(x, y);
	if (was_visible)
	{
		//fprintf(stderr, "was_visible\n");
		show();
	}
}

int gControl::scrollX()
{
	if (!_scroll)
		return 0;

	return (int)gtk_adjustment_get_value(gtk_scrolled_window_get_hadjustment(_scroll));
}

int gControl::scrollY()
{
	if (!_scroll)
		return 0;

	return (int)gtk_adjustment_get_value(gtk_scrolled_window_get_vadjustment(_scroll));
}

void gControl::setScrollX(int vl)
{
	GtkAdjustment* adj;
	int max;

	if (!_scroll)
		return;

	adj = gtk_scrolled_window_get_hadjustment(_scroll);

	max = (int)(gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj));

	if (vl < 0)
		vl = 0;
	else if (vl > max)
		vl = max;

	gtk_adjustment_set_value(adj, (gdouble)vl);
}

void gControl::setScrollY(int vl)
{
	GtkAdjustment* adj;
	int max;

	if (!_scroll)
		return;

	adj = gtk_scrolled_window_get_vadjustment(_scroll);

	max = (int)(gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj));

	if (vl < 0)
		vl = 0;
	else if (vl > max)
		vl = max;

	gtk_adjustment_set_value(adj, (gdouble)vl);
}

void gControl::scroll(int x, int y)
{
	setScrollX(x);
	setScrollY(y);
}

/*int gControl::scrollWidth()
{
	return widget->requisition.width;
}

int gControl::scrollHeight()
{
	return widget->requisition.height;
}*/

void gControl::setScrollBar(int vl)
{
	if (!_scroll)
		return;

	_scrollbar = vl & 3;
	updateScrollBar();
}

void gControl::updateScrollBar()
{
	if (!_scroll)
		return;
	
	switch(_scrollbar)
	{
		case SCROLL_NONE:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			break;
		case SCROLL_HORIZONTAL:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
			break;
		case SCROLL_VERTICAL:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
			break;
		case SCROLL_BOTH:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			break;
	}
}

int gControl::minimumHeight() const
{
	return 0;
}

int gControl::minimumWidth() const
{
	return 0;
}

bool gControl::isTracking() const
{
	if (_proxy)
		return _proxy->isTracking();
	else
		return _tracking;
}

void gControl::setTracking(bool v)
{
	if (_proxy)
		_proxy->setTracking(v);
	else
		_tracking = v;
	/*
	GtkWidget *wid;

	if (GTK_IS_EVENT_BOX(border))
		wid = border;
	else
		wid = widget;

	if (v != _tracking)
	{
		uint event_mask = gtk_widget_get_events(wid);
		_tracking = v;
		if (v)
		{
			_old_tracking = event_mask & GDK_POINTER_MOTION_MASK;
			event_mask |= GDK_POINTER_MOTION_MASK;
		}
		else
		{
			event_mask &= ~GDK_POINTER_MOTION_MASK;
		}

		if (!_old_tracking)
		{
			gtk_widget_unrealize(wid);
			gtk_widget_set_events(wid, event_mask);
			gtk_widget_realize(wid);
		}
	}
	*/
}

bool gControl::grab()
{
	gControl *old_control_grab;
	bool save_tracking;

	if (_grab)
		return false;

	if (gt_grab(border, FALSE, gApplication::lastEventTime()))
		return true;

	_grab = true;
	save_tracking = _tracking;
	_tracking = true;

	old_control_grab = gApplication::_control_grab;
	gApplication::_control_grab = this;

	gApplication::enterLoop(this);

	gApplication::_control_grab = old_control_grab;

	gt_ungrab();

	_tracking = save_tracking;
	_grab = false;
	return false;
}

bool gControl::hovered()
{
	//int x, y, xm, ym;

	if (!isVisible())
		return false;
	else
		return _inside;

	/*getScreenPos(&x, &y);
	gMouse::getScreenPos(&xm, &ym);

	return (xm >= x && ym >= y && xm < (x + width()) && ym < (y + height()));*/
}

bool gControl::setProxy(gControl *proxy)
{
	gControl *check = proxy;

	while (check)
	{
		if (check == this)
			return true;

		check = check->_proxy;
	}

	if (_proxy)
		_proxy->_proxy_for = NULL;

	_proxy = proxy;

	if (_proxy)
		_proxy->_proxy_for = this;

	return false;
}

void gControl::setNoTabFocus(bool v)
{
	if (_no_tab_focus == v)
		return;

	_no_tab_focus = v;
	if (pr)
		pr->updateFocusChain();
}

void gControl::emitEnterEvent(bool no_leave)
{
	gContainer *cont;

	//fprintf(stderr, "start enter %s\n", name());
	
	if (parent())
		parent()->emitEnterEvent(true);

	if (!no_leave && isContainer())
	{
		cont = (gContainer *)this;
		int i;

		for (i = 0; i < cont->childCount(); i++)
			cont->child(i)->emitLeaveEvent();
	}

	gApplication::_enter = this;
	
	if (gApplication::_leave)
	{
		if (gApplication::_leave == this || gApplication::_leave->isAncestorOf(this))
			gApplication::_leave = NULL;
	}
	
	if (_inside)
		return;
	_inside = true;

	//fprintf(stderr, "end enter %s\n", name());
	
	setMouse(mouse());

	if (gApplication::_ignore_until_next_enter)
	{
		//fprintf(stderr, "ignore next enter for %s\n", name());
		if (gApplication::_ignore_until_next_enter == this)
			gApplication::_ignore_until_next_enter = NULL;
		return;
	}

	//fprintf(stderr, "RAISE ENTER: %s\n", name());
	emit(SIGNAL(onEnterLeave), gEvent_Enter);
}

void gControl::emitLeaveEvent()
{
	if (gApplication::_enter == this)
		gApplication::_enter = NULL;
					
	if (!_inside)
		return;

	//fprintf(stderr, "start leave %s\n", name());
	
	if (isContainer())
	{
		gContainer *cont = (gContainer *)this;
		int i;

		for (i = 0; i < cont->childCount(); i++)
			cont->child(i)->emitLeaveEvent();
	}

	_inside = false;

	//fprintf(stderr, "end leave %s\n", name());
	
	if (parent()) parent()->setMouse(parent()->mouse());

	if (gApplication::_ignore_until_next_enter)
	{
		//fprintf(stderr, "ignore next leave for %s\n", name());
		return;
	}

	//fprintf(stderr, "RAISE LEAVE: %s\n", name());
	emit(SIGNAL(onEnterLeave), gEvent_Leave);
}

bool gControl::isAncestorOf(gControl *child)
{
	if (!isContainer())
		return false;

	for(;;)
	{
		child = child->parent();
		if (!child)
			return false;
		else if (child == this)
			return true;
	}
}

#ifdef GTK3
void gControl::drawBackground(cairo_t *cr)
{
	if (background() == COLOR_DEFAULT)
		return;

	//fprintf(stderr, "gControl::drawBackground\n");

	gt_cairo_set_source_color(cr, background());
	cairo_rectangle(cr, 0, 0, width(), height());
	cairo_fill(cr);
}
#else
void gControl::drawBackground(GdkEventExpose *e)
{
	GtkAllocation a;

	if (background() == COLOR_DEFAULT)
		return;

	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(border));

	gdk_cairo_region(cr, e->region);
	cairo_clip(cr);
	gt_cairo_set_source_color(cr, background());

	gtk_widget_get_allocation(border, &a);
	cairo_rectangle(cr, a.x, a.y, width(), height());
	cairo_fill(cr);

	cairo_destroy(cr);
}
#endif

bool gControl::canFocus() const
{
#if GTK_CHECK_VERSION(2, 18, 0)
	return gtk_widget_get_can_focus(widget);
#else
	return GTK_WIDGET_CAN_FOCUS(widget);
#endif
}

void gControl::setCanFocus(bool vl)
{
	if (vl == canFocus())
		return;
	
	gtk_widget_set_can_focus(widget, vl);
	
	/*_has_input_method = vl;
	
	if (_input_method && !vl)
	{
		g_object_unref(_input_method);
		_input_method = NULL;
	}
	else if (!_input_method && vl)
	{
		_input_method = gtk_im_multicontext_new();
	}*/
	
	if (pr)
		pr->updateFocusChain();
}

#ifdef GTK3
void gControl::updateColor()
{
}

void gControl::setColorNames(const char *bg_names[], const char *fg_names[])
{
	_bg_name_list = bg_names;
	_fg_name_list = fg_names;

	if (!bg_names)
	{
		_bg_name = NULL;
		_fg_name = NULL;
		use_base = FALSE;
		return;
	}

	gt_style_lookup_color(gtk_widget_get_style_context(widget), bg_names, &_bg_name, &_bg_default);
	gt_style_lookup_color(gtk_widget_get_style_context(widget), fg_names, &_fg_name, &_fg_default);
}

void gControl::setColorBase()
{
	static const char *bg_names[] = { "base_color", "theme_base_color", NULL };
	static const char *fg_names[] = { "text_color", "theme_text_color", NULL };
	setColorNames(bg_names, fg_names);
	use_base = TRUE;
}

void gControl::setColorButton()
{
	const char *bg_names[] = { "button_bg_color", "theme_button_bg_color", "theme_bg_color", NULL };
	const char *fg_names[] = { "button_fg_color", "theme_button_fg_color", "theme_fg_color", NULL };
	setColorNames(bg_names, fg_names);
	use_base = FALSE;
}
#endif

GtkIMContext *gControl::getInputMethod()
{
	return _input_method;
}
