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
#include "widgets_private.h"
#include "gapplication.h"
#include "gbutton.h"
#include "gdrawingarea.h"
#include "gmainwindow.h"
#include "gmoviebox.h"
#include "gpicturebox.h"
#include "gplugin.h"
#include "gscrollbar.h"
#include "gslider.h"
#include "gsplitter.h"
#include "gdesktop.h"
#include "gdrag.h"
#include "gmouse.h"
#include "gdraw.h"
#include "gcontrol.h"

static GList *controls=NULL;
static GList *controls_destroyed=NULL;

static const gchar _cursor_fdiag[] = {
   0x3f, 0x00, 0x1f, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x13, 0x00, 0x21, 0x00,
   0x40, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x84, 0x00, 0xc8,
   0x00, 0xf0, 0x00, 0xf0, 0x00, 0xf8, 0x00, 0xfc };
   
static const gchar _cursor_bdiag[] = {
   0x00, 0xfc, 0x00, 0xf8, 0x00, 0xf0, 0x00, 0xf0, 0x00, 0xc8, 0x00, 0x84,
   0x00, 0x02, 0x00, 0x01, 0x80, 0x00, 0x40, 0x00, 0x21, 0x00, 0x13, 0x00,
   0x0f, 0x00, 0x0f, 0x00, 0x1f, 0x00, 0x3f, 0x00 };
   
static const gchar _cursor_splith[] = {
   0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x64, 0x26, 0x66, 0x66,
   0x67, 0xe6, 0x7f, 0xfe, 0x7f, 0xfe, 0x67, 0xe6, 0x66, 0x66, 0x64, 0x26,
   0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06 };
   
static const gchar _cursor_splitv[] = {
   0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f, 0x80, 0x01, 0x80, 0x01, 0xff, 0xff,
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x80, 0x01,
   0x80, 0x01, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03 };


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
	GdkWindow *win = GTK_SOCKET(widget)->plug_window;
	if (!win)
		return 0;
	else
		return (int)GDK_WINDOW_XID(win);
}

void gPlugin::plug(int id, bool prepared)
{
	void (*func)(gControl *);
	int i;

	func = onPlug;
	onPlug = NULL;

	for (i = 1; i >= 0; i--)
	{
		if (i == 0)
			onPlug = func;
			
		if (!prepared) 
			gtk_socket_steal(GTK_SOCKET(widget), (GdkNativeWindow)id);
		else 
			gtk_socket_add_id(GTK_SOCKET(widget), (GdkNativeWindow)id);
	}
	
	if (client())
    XAddToSaveSet(gdk_display, client());
	else
		emit(SIGNAL(onError));
}

void gPlugin::discard()
{
	#ifdef GAMBAS_DIRECTFB
	stub("DIRECTFB/gPlugin:discard()");
	#else
	#ifdef GDK_WINDOWING_X11
	
	//gColor color;
	
	if (!client()) return;
	
  XRemoveFromSaveSet(gdk_display, client());
	XReparentWindow(gdk_display, client(), GDK_ROOT_WINDOW(), 0, 0);
	
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
	fnt = NULL;
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
	_font_set = false;
	have_cursor = false;
	use_base = false;
	mous = CURSOR_DEFAULT;
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
	
	/* Useless, as the control won't raise events anymore apparently.
	if (gApplication::_old_active_control == this)
	{
		if (onFocusEvent) 
		{
			fprintf(stderr, "focus out ??\n");
			onFocusEvent(this, gEvent_FocusOut);
		}
		gApplication::_old_active_control = NULL;
	}*/
	
	emit(SIGNAL(onFinish));
	
	if (win && win->focus == this)
		win->focus = 0;
	
	if (_proxy)
		_proxy->_proxy_for = NULL;
	if (_proxy_for)
		_proxy_for->_proxy = NULL;
	
	if (gDrag::getSource() == this)
		gDrag::cancel();
	
	if (curs) { delete curs; curs=NULL; }
	gFont::assign(&fnt);
	setName(NULL);

	controls = g_list_remove(controls, this);
	controls_destroyed = g_list_remove(controls_destroyed, this);
	
	if (gApplication::_enter == this)
		gApplication::_enter = NULL;
	if (gApplication::_leave == this)
		gApplication::_leave = NULL;
	if (gApplication::_active_control == this)
		gApplication::_active_control = NULL;
	if (gApplication::_old_active_control == this)
		gApplication::_old_active_control = NULL;
	if (gApplication::_button_grab == this)
		gApplication::_button_grab = NULL;
	if (gApplication::_control_grab == this)
		gApplication::_control_grab = NULL;
	if (gApplication::_ignore_until_next_enter == this)
		gApplication::_ignore_until_next_enter = NULL;
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


bool gControl::enabled()
{
	return GTK_WIDGET_SENSITIVE(border);
}

bool gControl::isReallyVisible()
{
	return GTK_WIDGET_MAPPED(border);
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
	if (!border->window)
	{
		*x = *y = 0;
		return;
	}
	
	gdk_window_get_origin(border->window, x, y);
	
	#if GTK_CHECK_VERSION(2, 18, 0)
	if (!gtk_widget_get_has_window(border))
	{
		*x += border->allocation.x;
		*y += border->allocation.y;
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

	if (!GTK_WIDGET_REALIZED(widget))
		return;
	
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
	
	if (pr && pr->getClass() == Type_gSplitter) 
		return;

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
	if (pr && pr->getClass() == Type_gSplitter) 
		return;
	
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
	if (fnt)
		return fnt;
	if (pr)
		return pr->font();
	
	return gDesktop::font();
}


void gControl::resolveFont(gFont *new_font)
{
	gFont *font = new gFont();
	gControl *ctrl = this;
	
	font->mergeFrom(new_font);

	for(;;)
	{
		if (font->isAllSet())
			break;

		ctrl = ctrl->parent();
		if (!ctrl)
			break;

		if (ctrl->fnt)
			font->mergeFrom(ctrl->fnt);
	}
	
	gtk_widget_modify_font(widget, font->desc());
	
	gFont::set(&fnt, font);
}

void gControl::setFont(gFont *ft)
{
	if (ft)
	{
		resolveFont(ft);
		_font_set = true;
	}
	else if (fnt)
	{
		gFont::assign(&fnt);
		gtk_widget_modify_font(widget, NULL);
		_font_set = false;
	}
	
	resize();
}


int gControl::mouse()
{
	if (_proxy)
		return _proxy->mouse();
	else
		return mous;
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
  if (GDK_IS_WINDOW(border->window) && _inside)
    gdk_window_set_cursor(border->window, cursor);
}

void gControl::setMouse(int m)
{
	GdkCursor *cr = NULL;
	GdkPixmap *pix;
	GdkColor col = {0,0,0,0};
	
	if (_proxy)
	{
		_proxy->setMouse(m);
		return;
	}
	
	mous = m;
	
	if (gApplication::isBusy())
    m = GDK_WATCH;
	
	if (m == CURSOR_CUSTOM)
	{
		if (!curs || !curs->cur)
		{
			mous = CURSOR_DEFAULT;
			updateCursor(NULL);
		}
		else
			updateCursor(curs->cur);
		
		return;	
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
				pix = gdk_bitmap_create_from_data(NULL,_cursor_fdiag,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m == (GDK_LAST_CURSOR+2)) //BDiag
			{
				pix = gdk_bitmap_create_from_data(NULL,_cursor_bdiag,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m == (GDK_LAST_CURSOR+3)) //SplitH
			{
				pix = gdk_bitmap_create_from_data(NULL,_cursor_splith,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
			else if (m == (GDK_LAST_CURSOR+4)) //SplitV
			{
				pix = gdk_bitmap_create_from_data(NULL,_cursor_splitv,16,16);
				cr = gdk_cursor_new_from_pixmap(pix,pix,&col,&col,0,0);
				g_object_unref(pix);
			}
		}
	}
	
  updateCursor(cr);
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
	if (!border->window)
		return 0;
	else
		return GDK_WINDOW_XID(border->window);
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
		
		r.x = border->allocation.x + x;
		r.y = border->allocation.y + y;
		r.width = w;
		r.height = h;
  
		gdk_window_invalidate_rect(border->window, &r, TRUE);
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
		return (border && GTK_WIDGET_HAS_FOCUS(border)) || (widget && GTK_WIDGET_HAS_FOCUS(widget)) || gApplication::activeControl() == this;
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
	if (pr->getClass()==Type_gSplitter) return;
	
	if (gtk_widget_get_has_window(border))
	{
		gdk_window_lower(border->window);
		if (widget->window)
			gdk_window_lower(widget->window);
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
	if (pr->getClass()==Type_gSplitter) return;
	
	if (gtk_widget_get_has_window(border))
	{
		gdk_window_raise(border->window);
		if (widget->window)
			gdk_window_raise(widget->window);
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
		stack[0] = GDK_WINDOW_XID(ctrl->border->window);
		stack[1] = GDK_WINDOW_XID(border->window);
		
		XRestackWindows(GDK_WINDOW_XDISPLAY(border->window), stack, 2 );
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

void gControl::drawBorder(GdkEventExpose *e)
{
	GdkDrawable *win;
	GtkShadowType shadow;
	gint x, y, w, h;
	//GdkGC *gc;
	GtkStyle* st;
	GdkRectangle clip;
	
	if (getFrameBorder() == BORDER_NONE)
    return;
	
	x = 0;
	y = 0;
  w = width();
  h = height();
  
	//if (!win)
	{
		GtkWidget *wid;
		
		if (frame)
			wid = frame;
		else
			wid = widget;
			
		if (GTK_IS_LAYOUT(wid))
			win = GTK_LAYOUT(wid)->bin_window;
		else
			win = wid->window;
		
		x = wid->allocation.x;
		y = wid->allocation.y;
	}
	
  if (w < 1 || h < 1)
  	return;
  
	st = gtk_widget_get_style(widget);
  
	switch (getFrameBorder())
	{
    case BORDER_PLAIN:
    {
			GdkGC *gc;
			GdkGCValues values;

			fill_gdk_color(&values.foreground, getFrameColor(), gdk_drawable_get_colormap(win));
			gc = gtk_gc_get(gdk_drawable_get_depth(win), gdk_drawable_get_colormap(win), &values, GDK_GC_FOREGROUND);
			
			//gdk_draw_rectangle(win, use_base ? st->text_gc[GTK_STATE_NORMAL] : st->fg_gc[GTK_STATE_NORMAL], FALSE, x, y, w - 1, h - 1); 
			gdk_gc_set_clip_region(gc, e->region);
			gdk_draw_rectangle(win, gc, FALSE, x, y, w - 1, h - 1); 
			gtk_gc_release(gc);
      return;
    }
    
    case BORDER_SUNKEN: shadow = GTK_SHADOW_IN; break;
    case BORDER_RAISED: shadow = GTK_SHADOW_OUT; break;
    case BORDER_ETCHED: shadow = GTK_SHADOW_ETCHED_IN; break;
    
    default: 
      return;
	}
	
	gdk_region_get_clipbox(e->region, &clip);
  gtk_paint_shadow(st, win, GTK_STATE_NORMAL, shadow, &clip, NULL, NULL, x, y, w, h);
}

static gboolean cb_frame_expose(GtkWidget *wid, GdkEventExpose *e, gControl *control)
{
	control->drawBorder(e);
	return false;
}

static gboolean cb_draw_background(GtkWidget *wid, GdkEventExpose *e, gControl *control)
{
	control->drawBackground(e);
	return false;
}

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

static gboolean cb_clip_children(GtkWidget *wid, GdkEventExpose *e, gContainer *d)
{
	GdkRegion *me;
	
	me = gdk_region_rectangle((GdkRectangle *)&wid->allocation);
	
	gdk_region_intersect(e->region, me);
	
	gdk_region_destroy(me);
	
	if (gdk_region_empty(e->region))
		return TRUE;
	
	return FALSE;
}

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

	connectParent();
	initSignals();
	
  if (frame)
		g_signal_connect_after(G_OBJECT(frame), "expose-event", G_CALLBACK(cb_frame_expose), (gpointer)this);
	
	if (!_no_background && !gtk_widget_get_has_window(border))
		g_signal_connect(G_OBJECT(border), "expose-event", G_CALLBACK(cb_draw_background), (gpointer)this);
	/*else if (!isTopLevel())
	{
		fprintf(stderr, "clip by parent\n");
		g_signal_connect(G_OBJECT(border), "expose-event", G_CALLBACK(cb_clip_by_parent), (gpointer)this);
	}*/
	
	if (isContainer() && !gtk_widget_get_has_window(widget))
		g_signal_connect(G_OBJECT(widget), "expose-event", G_CALLBACK(cb_clip_children), (gpointer)this);
	
	//if (isContainer() && widget != border)
	//	g_signal_connect(G_OBJECT(widget), "size-allocate", G_CALLBACK(cb_size_allocate), (gpointer)this);
	
	gtk_widget_add_events(widget, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
		| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	if (widget != border && (GTK_IS_WINDOW(border) || (GTK_IS_EVENT_BOX(border) && !gtk_event_box_get_visible_window(GTK_EVENT_BOX(border)))))
	{
		gtk_widget_add_events(border, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
			| GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	}
		
	registerControl();
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
		return use_base ? get_gdk_base_color(widget, enabled()) : get_gdk_bg_color(widget, enabled());
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
		return use_base ? get_gdk_text_color(widget, enabled()) : get_gdk_fg_color(widget, enabled());
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
	
	return (int)gtk_scrolled_window_get_hadjustment(_scroll)->value;
}

int gControl::scrollY()
{
	if (!_scroll)
		return 0;
	
	return (int)gtk_scrolled_window_get_vadjustment(_scroll)->value;
}

void gControl::setScrollX(int vl)
{
	GtkAdjustment* adj;
	int max;
	
	if (!_scroll)
		return;
		
	adj = gtk_scrolled_window_get_hadjustment(_scroll);
	
	max = (int)(adj->upper - adj->page_size);
	
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
	
	max = (int)(adj->upper - adj->page_size);
	
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

int gControl::scrollWidth()
{
	/*if (widget->requisition.width <= 0)
	{
		gtk_widget_realize(widget);
		gtk_widget_size_request(widget, &widget->requisition);
	}*/
	return widget->requisition.width;
}

int gControl::scrollHeight()
{
	/*if (widget->requisition.height <= 0)
	{
		gtk_widget_realize(widget);
		gtk_widget_size_request(widget, &widget->requisition);
	}*/
	return widget->requisition.height;
}

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
	
	win = border->window;
	
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

void gControl::drawBackground(GdkEventExpose *e)
{
	if (background() == COLOR_DEFAULT)
		return;
	
	//fprintf(stderr, "drawBackground: %s %08X\n", name(), background());
					
	gDraw *d = new gDraw();
	d->connect(this);
	d->setFillStyle(FILL_SOLID);
	d->setFillColor(background());
	d->setLineStyle(LINE_NONE);
	gdk_gc_set_clip_region(d->getGC(), e->region);
	d->rect(0, 0, width(), height());
	delete d;
}

bool gControl::canFocus() const
{
	return GTK_WIDGET_CAN_FOCUS(widget);
}

void gControl::setCanFocus(bool vl)
{
	gtk_widget_set_can_focus(widget, vl);
}

