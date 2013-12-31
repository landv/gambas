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
#include "gpicturebox.h"
#include "gplugin.h"
#include "gscrollbar.h"
#include "gslider.h"
#include "gdesktop.h"
#include "gdrag.h"
#include "gmouse.h"
#include "gcontrol.h"

static GList *controls=NULL;
static GList *controls_destroyed=NULL;


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


// Geometry optimization hack - Sometimes fails, so it is disabled...
#define GEOMETRY_OPTIMIZATION 0

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

	border=gtk_socket_new();
	widget=border;
	realize(false);

	onPlug=NULL;
	onUnplug=NULL;

	g_signal_connect(G_OBJECT(widget),"plug-removed",G_CALLBACK(gPlugin_OnUnplug),(gpointer)this);
	g_signal_connect(G_OBJECT(widget),"plug-added",G_CALLBACK(gPlugin_OnPlug),(gpointer)this);

	g_object_set(G_OBJECT(widget),"can-focus",TRUE, (void *)NULL);
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
	#ifdef GAMBAS_DIRECTFB
	stub("DIRECTFB/gPlugin:discard()");
	#else
	#ifdef GDK_WINDOWING_X11

	Display *d = gdk_x11_display_get_xdisplay(gdk_display_get_default());

	if (!client()) return;

	XRemoveFromSaveSet(d, client());
	XReparentWindow(d, client(), GDK_ROOT_WINDOW(), 0, 0);

	#else
	stub("no-X11/gPlugin:discard()");
	#endif
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
		//controls_destroyed = g_list_remove(controls_destroyed, (gpointer)control);
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
	no_input_method = false;
	_no_default_mouse_event = false;
	_proxy = _proxy_for = NULL;
	_no_tab_focus = false;
	_inside = false;
	_no_auto_grab = false;
	_no_background = false;

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

	setName(NULL);

	controls = g_list_remove(controls, this);
	controls_destroyed = g_list_remove(controls_destroyed, this);

	#define CLEAN_POINTER(_p) if (_p == this) _p = NULL

	CLEAN_POINTER(gApplication::_enter);
	CLEAN_POINTER(gApplication::_leave);
	CLEAN_POINTER(gApplication::_active_control);
	CLEAN_POINTER(gApplication::_old_active_control);
	CLEAN_POINTER(gApplication::_button_grab);
	CLEAN_POINTER(gApplication::_control_grab);
	CLEAN_POINTER(gApplication::_ignore_until_next_enter);
}

void gControl::destroy()
{
	if (_destroyed)
		return;

	hide();

	//fprintf(stderr, "added to destroy list: %p\n", this);
	controls_destroyed = g_list_prepend(controls_destroyed,(gpointer)this);
	_destroyed = true;

	if (pr)
		pr->remove(this);
}


bool gControl::isEnabled() const
{
	return gtk_widget_is_sensitive(border);
}

bool gControl::isReallyVisible()
{
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

void gControl::getScreenPos(int *x, int *y)
{
	if (!gtk_widget_get_window(border))
	{
		*x = *y = 0;
		return;
	}

	gdk_window_get_origin(gtk_widget_get_window(border), x, y);

	#if GTK_CHECK_VERSION(2, 18, 0)
	if (!gtk_widget_get_has_window(border))
	{
		GtkAllocation a;
		gtk_widget_get_allocation(border, &a);
		*x += a.x;
		*y += a.y;
	}
	#endif
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

	send_configure(this);
}

void gControl::resize(int w, int h)
{
	if (w < 1)
		w = 0;

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

	send_configure(this);
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
		if (_dirty_size)
		{
			//GtkAllocation a = { x(), y(), width(), height() };
			//gtk_widget_size_allocate(border, &a);
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

char* gControl::toolTip()
{
	char *text = gtk_widget_get_tooltip_text(border);
	gt_free_later(text);
	return text;
}

void gControl::setToolTip(char* vl)
{
	gtk_widget_set_tooltip_text(border, vl ? vl : "");
}

gFont* gControl::font()
{
	if (_resolved_font)
	{
		//fprintf(stderr, "%p: font -> _resolved_font\n", this);
		return _resolved_font;
	}
	else if (pr)
	{
		//fprintf(stderr, "%p: font -> parent\n", this);
		return pr->font();
	}
	else
	{
		//fprintf(stderr, "%p: font -> desktop\n", this);
		return gDesktop::font();
	}
}

void gControl::actualFontTo(gFont *ft)
{
	//fprintf(stderr, "actualFontTo: %s: %s / %s (_font = %s)\n", name(), ft->toString(), ft->toFullString(), _font ? _font->toFullString() : NULL);
	font()->copyTo(ft);
	ft->setAllFrom(_font);
	//fprintf(stderr, "==> %s: %s / %s (_font = %s)\n", name(), ft->toString(), ft->toFullString(), _font ? _font->toFullString() : NULL);
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
	GdkPixbuf *pix;
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
		if (m < GDK_LAST_CURSOR)
		{
			cr = gdk_cursor_new((GdkCursorType)m);
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
		}
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
	#ifndef GAMBAS_DIRECTFB
	#ifdef GDK_WINDOWING_X11
	if (!gtk_widget_get_window(border))
		return 0;
	else
		return GDK_WINDOW_XID(gtk_widget_get_window(border));
	#else
	stub("no-X11/gControl::handle()");
	return 0;
	#endif
	#else
	stub("DIRECTFB/gControl::handle()");
	return 0;
	#endif

}

/*****************************************************************

MISC

******************************************************************/

void gControl::refresh()
{
	gtk_widget_queue_draw(border);
	if (frame != border && GTK_IS_WIDGET(frame))
		gtk_widget_queue_draw(frame);
	if (widget != frame  && GTK_IS_WIDGET(widget))
		gtk_widget_queue_draw(widget);

	afterRefresh();
}

void gControl::refresh(int x, int y, int w, int h)
{
	if (x < 0 || y < 0 || w < 0 || h < 0)
		gtk_widget_queue_draw(border);
	else
	{
		// Buggy GTK+
		// gtk_widget_queue_draw_area(border, x, y, w, h);
		GdkRectangle r;
		GtkAllocation a;

		gtk_widget_get_allocation(border, &a);

		r.x = a.x + x;
		r.y = a.y + y;
		r.width = w;
		r.height = h;

		gdk_window_invalidate_rect(gtk_widget_get_window(border), &r, TRUE);
	}

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

bool gControl::hasFocus()
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

	if (gtk_widget_get_has_window(border))
	{
		gdk_window_lower(gtk_widget_get_window(border));
		if (gtk_widget_get_window(widget))
			gdk_window_lower(gtk_widget_get_window(widget));
	}
	else
	{
		//fprintf(stderr, "gb.gtk: warning: gControl::lower(): no window\n");

		if (!(chd=gtk_container_get_children(GTK_CONTAINER(pr->getContainer())))) return;

		chd = g_list_first(chd);

		while(chd)
		{
			child = (GtkWidget*)chd->data;

			br = (gControl *)g_object_get_data(G_OBJECT(child), "gambas-control");;

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

	if (gtk_widget_get_has_window(border))
	{
		gdk_window_raise(gtk_widget_get_window(border));
		if (gtk_widget_get_window(widget))
			gdk_window_raise(gtk_widget_get_window(widget));
	}
	else
	{
		//fprintf(stderr, "gb.gtk: warning: gControl::raise(): no window\n");

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
	}

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
	gt_draw_border(cr, gtk_widget_get_style_context(widget), GTK_STATE_FLAG_NORMAL, getFrameBorder(), getFrameColor(), 0, 0, width(), height());
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
	gtk_paint_shadow(st, win, GTK_STATE_NORMAL, shadow, &clip, NULL, NULL, x, y, w, h);
}
#endif

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
	g_object_set_data(G_OBJECT(border), "gambas-control", this);
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

#define PATCH_DECLARE(type) \
static void type##_get_preferred_width(GtkWidget *widget, gint *minimum_size, gint *natural_size) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass*)g_type_class_peek(type); \
	\
	(*(void (*)(GtkWidget *, gint *, gint *))klass->_gtk_reserved2)(widget, minimum_size, natural_size); \
	if (minimum_size && g_object_get_data(G_OBJECT(widget), "gambas-control")) \
		*minimum_size = 0; \
} \
static void type##_get_preferred_height(GtkWidget *widget, gint *minimum_size, gint *natural_size) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)g_type_class_peek(type); \
	\
	(*(void (*)(GtkWidget *, gint *, gint *))klass->_gtk_reserved3)(widget, minimum_size, natural_size); \
	if (minimum_size && g_object_get_data(G_OBJECT(widget), "gambas-control")) \
		*minimum_size = 0; \
}

//fprintf(stderr, "patching [%p %s] (%p %p)\n", klass, G_OBJECT_TYPE_NAME(border), klass->get_preferred_width, klass->get_preferred_height);

#define PATCH_CLASS(widget, type) \
if (G_OBJECT_TYPE(widget) == type) \
{ \
	GtkWidgetClass *klass = (GtkWidgetClass *)GTK_WIDGET_GET_CLASS(widget); \
	if (klass->get_preferred_width != type##_get_preferred_width) \
	{ \
		klass->_gtk_reserved2 = (void (*)())klass->get_preferred_width; \
		klass->get_preferred_width = type##_get_preferred_width; \
		klass->_gtk_reserved3 = (void (*)())klass->get_preferred_height; \
		klass->get_preferred_height = type##_get_preferred_height; \
	} \
}

PATCH_DECLARE(GTK_TYPE_WINDOW)
PATCH_DECLARE(GTK_TYPE_ENTRY)
PATCH_DECLARE(GTK_TYPE_SPIN_BUTTON)
PATCH_DECLARE(GTK_TYPE_BUTTON)
PATCH_DECLARE(GTK_TYPE_FIXED)
PATCH_DECLARE(GTK_TYPE_EVENT_BOX)
PATCH_DECLARE(GTK_TYPE_ALIGNMENT)
PATCH_DECLARE(GTK_TYPE_TOGGLE_BUTTON)
PATCH_DECLARE(GTK_TYPE_SCROLLED_WINDOW)
PATCH_DECLARE(GTK_TYPE_CHECK_BUTTON)
PATCH_DECLARE(GTK_TYPE_RADIO_BUTTON)

int gt_get_preferred_width(GtkWidget *widget)
{
	int m, n;
	GtkWidgetClass *klass = (GtkWidgetClass*)g_type_class_peek(G_OBJECT_TYPE(widget));
	if (klass->_gtk_reserved2)
		(*(void (*)(GtkWidget *, gint *, gint *))klass->_gtk_reserved2)(widget, &m, &n);
	return m;
}

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
			frame = gtk_alignment_new(0, 0, 1, 1);
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

#ifdef GTK3

	PATCH_CLASS(border, GTK_TYPE_WINDOW)
	else PATCH_CLASS(border, GTK_TYPE_ENTRY)
	else PATCH_CLASS(border, GTK_TYPE_SPIN_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_FIXED)
	else PATCH_CLASS(border, GTK_TYPE_EVENT_BOX)
	else PATCH_CLASS(border, GTK_TYPE_ALIGNMENT)
	else PATCH_CLASS(border, GTK_TYPE_TOGGLE_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_SCROLLED_WINDOW)
	else PATCH_CLASS(border, GTK_TYPE_CHECK_BUTTON)
	else PATCH_CLASS(border, GTK_TYPE_RADIO_BUTTON)
	else fprintf(stderr, "gb.gtk3: warning: class %s was not patched\n", G_OBJECT_TYPE_NAME(border));

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

	border = GTK_WIDGET(_scroll);
	widget = wid;
	frame = 0;
	_no_auto_grab = true;

	//gtk_container_add(GTK_CONTAINER(border), GTK_WIDGET(_scroll));
	gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(_scroll), widget);

	if (!doNotRealize)
		realize(false);
	else
		registerControl();

	updateFont();
}

void gControl::updateBorder()
{
	int padding;

	if (!frame)
		return;

	if (!GTK_IS_ALIGNMENT(frame))
	{
		refresh();
		return;
	}

	switch (frame_border)
	{
		case BORDER_NONE: padding = 0; break;
		case BORDER_PLAIN: padding = 1; break;
		default: padding = 2; break; // TODO: Use style
	}

	if ((int)frame_padding > padding)
		padding = frame_padding;

	gtk_alignment_set_padding(GTK_ALIGNMENT(frame), padding, padding, padding, padding);
	refresh();
	//gtk_widget_queue_draw(frame);
}

int gControl::getFrameWidth()
{
	guint p;

	if (frame && GTK_IS_ALIGNMENT(frame))
	{
		gtk_alignment_get_padding(GTK_ALIGNMENT(frame), &p, NULL, NULL, NULL);
		return p;
	}

	if (_scroll)
	{
		if (gtk_scrolled_window_get_shadow_type(_scroll) == GTK_SHADOW_NONE)
			return 0;
		else
			return 2;
	}

	switch (frame_border)
	{
		case BORDER_NONE: p = 0; break;
		case BORDER_PLAIN: p = 1; break;
		default: p = 2; break; // TODO: Use style
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
	if (_scroll)
		return gtk_scrolled_window_get_shadow_type(_scroll) != GTK_SHADOW_NONE;
	else
		return getFrameBorder() != BORDER_NONE;
}

void gControl::setBorder(bool vl)
{
	if (_scroll)
	{
		if (vl)
			gtk_scrolled_window_set_shadow_type(_scroll, GTK_SHADOW_IN);
		else
			gtk_scrolled_window_set_shadow_type(_scroll, GTK_SHADOW_NONE);
	}
	else
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


gColor gControl::realBackground()
{
	if (_bg_set)
		return use_base ? get_gdk_base_color(widget, isEnabled()) : get_gdk_bg_color(widget, isEnabled());
	else if (pr)
		return pr->realBackground();
	else
		return gDesktop::bgColor();
}

gColor gControl::background()
{
	if (_bg_set)
		return realBackground();
	else
		return COLOR_DEFAULT;
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
	_bg_set = color != COLOR_DEFAULT;

	if (!_bg_set)
	{
		if (pr && !use_base)
			color = pr->realBackground();
	}

	setRealBackground(color);
}

gColor gControl::realForeground()
{
	if (_fg_set)
		return use_base ? get_gdk_text_color(widget, isEnabled()) : get_gdk_fg_color(widget, isEnabled());
	else if (pr)
		return pr->realForeground();
	else
		return gDesktop::fgColor();
}

gColor gControl::foreground()
{
	if (_fg_set)
		return realForeground();
	else
		return COLOR_DEFAULT;
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
	/*set_foreground(border, color, use_base);
	if (border != frame && GTK_IS_WIDGET(frame))
		set_foreground(frame, color, use_base);
	if (frame != widget)*/
}

void gControl::setForeground(gColor color)
{
	_fg_set = color != COLOR_DEFAULT;

	if (!_fg_set)
	{
		if (pr)
			color = pr->realForeground();
	}

	setRealForeground(color);
}

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

	if (was_visible) hide();
	//gtk_widget_unrealize(border);

	oldpr = pr;
	pr = newpr;

	if (oldpr == newpr)
	{
		gtk_widget_reparent(border, newpr->getContainer());
		oldpr->performArrange();
	}
	else
	{
		if (oldpr)
		{
			gtk_widget_reparent(border, newpr->getContainer());
			oldpr->remove(this);
			oldpr->performArrange();
		}

		newpr->insert(this);
	}

	//gtk_widget_realize(border);
	move(x, y);
	if (was_visible) show();
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

int gControl::scrollBar()
{
	GtkPolicyType h, v;
	int ret = 3;

	if (!_scroll)
		return 0;

	gtk_scrolled_window_get_policy(_scroll, &h, &v);
	if (h == GTK_POLICY_NEVER) ret--;
	if (v == GTK_POLICY_NEVER) ret -= 2;

	return ret;
}

void gControl::setScrollBar(int vl)
{
	if (!_scroll)
		return;

	switch(vl)
	{
		case 0:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			break;
		case 1:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
			break;
		case 2:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
			break;
		case 3:
			gtk_scrolled_window_set_policy(_scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			break;
	}
}

int gControl::minimumHeight()
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
	GdkWindow *win;
	gControl *old_control_grab;
	bool save_tracking;

	if (_grab)
		return false;

	win = gtk_widget_get_window(border);

	if (gdk_pointer_grab(win, FALSE, (GdkEventMask)(GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK), NULL,
	#if GTK_CHECK_VERSION(2, 18, 0)
			gdk_window_get_cursor(win),
	#else
			NULL,
	#endif
			gApplication::lastEventTime()) != GDK_GRAB_SUCCESS)
	{
		fprintf(stderr, "gb.gtk: warning: cannot grab pointer\n");
		return true;
	}
	if (gdk_keyboard_grab(win, FALSE, gApplication::lastEventTime()) != GDK_GRAB_SUCCESS)
	{
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		fprintf(stderr, "gb.gtk: warning: cannot grab keyboard\n");
		return true;
	}

	_grab = true;
	save_tracking = _tracking;
	_tracking = true;

	old_control_grab = gApplication::_control_grab;
	gApplication::_control_grab = this;

	gApplication::enterLoop(this);

	gApplication::_control_grab = old_control_grab;

	gdk_pointer_ungrab(GDK_CURRENT_TIME);
	gdk_keyboard_ungrab(GDK_CURRENT_TIME);

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

	if (parent())
		parent()->emitEnterEvent(true);

	if (!no_leave && isContainer())
	{
		cont = (gContainer *)this;
		int i;

		for (i = 0; i < cont->childCount(); i++)
			cont->child(i)->emitLeaveEvent();
	}

	if (_inside)
		return;

	_inside = true;

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
	if (!_inside)
		return;

	if (isContainer())
	{
		gContainer *cont = (gContainer *)this;
		int i;

		for (i = 0; i < cont->childCount(); i++)
			cont->child(i)->emitLeaveEvent();
	}

	_inside = false;

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
	gtk_widget_set_can_focus(widget, vl);
}

