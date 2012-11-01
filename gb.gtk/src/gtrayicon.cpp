/***************************************************************************

  gtrayicon.cpp

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

#include "widgets.h"

#ifdef GDK_WINDOWING_X11
#ifndef GAMBAS_DIRECTFB
#define GOT_TRAYICON
#endif
#endif

#ifdef GOT_TRAYICON
#include <unistd.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#endif
#include "x11.h"
#include "gapplication.h"
#include "gmouse.h"
#include "gtrayicon.h"

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define OPCODE "_NET_SYSTEM_TRAY_OPCODE"


/*****************************************************************************

Default picture

******************************************************************************/

#include "gb.form.trayicon.h"

/*****************************************************************************

Low level stuff

******************************************************************************/
#ifdef GOT_TRAYICON

void XTray_getSize(Display *xdisplay,Window icon,unsigned int *w,unsigned int *h)
{
	XWindowAttributes Attr;

	XGetWindowAttributes(xdisplay,icon,&Attr);

	if (w) *w=Attr.width;
	if (h) *h=Attr.height;

}

void XTray_getPosition(Display *xdisplay,Window icon,unsigned int *x,unsigned int *y)
{
	Window rwin;
	Window pwin;
	Window *cwin;
	unsigned int count;
	XWindowAttributes Attr;
	Window w=icon;

	if (x) *x=0;
	if (y) *y=0;

	do
	{
		XQueryTree(xdisplay,w,&rwin,&pwin,&cwin,&count);
		if (cwin) XFree(cwin);
		if (pwin)
		{
			XGetWindowAttributes(xdisplay, pwin,&Attr);
			if (x) (*x)+=Attr.x;
			if (y) (*y)+=Attr.y;
			w=pwin;
		}
	} while (pwin);
}

#endif


/*************************************************************************

gTrayIcon

**************************************************************************/

static gboolean  tray_enterleave(GtkWidget *widget, GdkEventCrossing *e,gTrayIcon *data)
{
	if (gApplication::loopLevel() > data->loopLevel()) return false;
	if (e->type==GDK_ENTER_NOTIFY)
	{
		if (data->onEnter) data->onEnter(data);
	}
	else
	{
		if (data->onLeave) data->onLeave(data);
	}
	return false;
}

static void tray_destroy (GtkWidget *object,gTrayIcon *data)
{
	data->cleanUp();
}

static gboolean tray_event(GtkWidget *widget, GdkEvent *event,gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	gApplication::updateLastEventTime(event);

	if (event->type==GDK_2BUTTON_PRESS)
	{
		if (data->onDoubleClick) data->onDoubleClick(data);
		return false;
	}

	return false;
}


static gboolean tray_down (GtkWidget *widget,GdkEventButton *event,gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	if (data->onMousePress)
	{
		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, (int)event->x_root, (int)event->y_root, event->button, event->state);
		data->onMousePress(data);
		gMouse::invalidate();
	}

	if (event->button==3)
		if (data->onMenu)
			data->onMenu(data);

	return false;
}

static gboolean tray_up (GtkWidget *widget,GdkEventButton *event,gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	if (data->onMouseRelease)
	{
		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, (int)event->x_root, (int)event->y_root, event->button, event->state);
		data->onMouseRelease(data);
		gMouse::invalidate();
	}

	return false;
}

static gboolean cb_menu(GtkWidget *widget, gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;
	
	if (data->onMenu)
		data->onMenu(data);
	
	return false;
}

static gboolean tray_focus_In(GtkWidget *widget,GdkEventFocus *event,gTrayIcon *data)
{
	if (!gApplication::allEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	if (data->onFocusEnter) data->onFocusEnter(data);
	return false;
}

static gboolean tray_focus_Out(GtkWidget *widget,GdkEventFocus *event,gTrayIcon *data)
{
	if (!gApplication::allEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	if (data->onFocusLeave) data->onFocusLeave(data);
	return false;
}

static gboolean cb_scroll(GtkWidget *widget, GdkEventScroll *event, gTrayIcon *data)
{
	int dt = 0;
	int ort = 0;
	
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	if (data->onMouseWheel)
	{
		switch (event->direction)
		{
			case GDK_SCROLL_UP: dt=1; ort=1; break;
			case GDK_SCROLL_DOWN: dt=-1; ort=1; break;
			case GDK_SCROLL_LEFT: dt=-1; ort=0; break;
			case GDK_SCROLL_RIGHT:  dt=1; ort=0; break;
		}
		
		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, (int)event->x_root, (int)event->y_root, 0, event->state);
		gMouse::setWheel(dt, ort);
		data->onMouseWheel(data);
		gMouse::invalidate();
	}
	
	return false;
}

static gboolean cb_expose(GtkWidget *widget, GdkEventExpose *e, gTrayIcon *data)
{
	gPicture *pic = data->getIcon();
	
	gdk_window_clear(widget->window);
	gdk_draw_pixbuf(widget->window,
			widget->style->black_gc,
			pic->getPixbuf(),
			0,
			0,
			(widget->allocation.width - pic->width()) / 2,
			(widget->allocation.height - pic->height()) / 2,
			-1,
			-1,
			GDK_RGB_DITHER_NORMAL,
			0, 0);
	
	return true;
}


GList *gTrayIcon::trayicons = NULL;
gPicture *gTrayIcon::_default_icon = NULL;

#ifdef GOT_TRAYICON

gTrayIcon::gTrayIcon()
{
	plug = NULL;
	buftext = NULL;
	_icon = NULL;
	_style = NULL;
	_loopLevel = 0;

	onMousePress=NULL;
	onMouseRelease=NULL;
	onMouseWheel = NULL;
	onMenu=NULL;
	onFocusEnter=NULL;
	onFocusLeave=NULL;
	onDoubleClick=NULL;
	onEnter=NULL;
	onLeave=NULL;
	
	trayicons = g_list_append(trayicons, (gpointer)this);
}

gTrayIcon::~gTrayIcon()
{
	gPicture::assign(&_icon);
	
	if (buftext) 
	{
		g_free(buftext);
		buftext = NULL;
	}
	
	if (plug) 
		gtk_widget_destroy(plug);
	
	setVisible(false);
	trayicons = g_list_remove(trayicons, (gpointer)this);
	
	if (!trayicons && _default_icon)
	{
		delete _default_icon;
		_default_icon = NULL;
	}

	if (_style)
		g_object_unref(_style);

	if (onDestroy) (*onDestroy)(this);
}

gPicture *gTrayIcon::defaultIcon()
{
	if (!_default_icon)
	{
		GdkPixbuf *img = gdk_pixbuf_new_from_xpm_data((const char**)_default_trayicon);
		_default_icon = new gPicture(img);
	}
	
	return _default_icon;
}

void gTrayIcon::refresh()
{
	if (plug)
		gtk_widget_queue_draw(plug);
}

void gTrayIcon::setPicture(gPicture *picture)
{
	gPicture::assign(&_icon, picture);
	refresh();
}

char* gTrayIcon::toolTip()
{
	return buftext;
}

void gTrayIcon::updateTooltip()
{
	if (!plug)
		return;
		
	gtk_widget_set_tooltip_text(plug, buftext);
}

void gTrayIcon::setToolTip(char* vl)
{
	if (buftext) 
		g_free(buftext);
		
	buftext = vl && *vl ? g_strdup(vl) : NULL;
	updateTooltip();
}

bool gTrayIcon::isVisible()
{
	return (bool)plug;
}

void gTrayIcon::cleanUp()
{
	if (plug)
	{
		plug = NULL;
		g_object_unref(_style);
		_style = NULL;
	}
}

void gTrayIcon::setVisible(bool vl)
{
	if (vl)
	{
		Window win;
		gPicture *pic;
	
		if (!plug)
		{
			_loopLevel = gApplication::loopLevel() + 1;
			
			plug = gtk_plug_new(0);
			gtk_widget_set_double_buffered(plug, FALSE);	
			
			_style = gtk_style_copy(GTK_WIDGET(plug)->style);
			_style->bg_pixmap[GTK_STATE_NORMAL] = (GdkPixmap*)GDK_PARENT_RELATIVE;
			gtk_widget_set_style(GTK_WIDGET(plug), _style);
			
			pic = getIcon();
			gtk_widget_set_size_request(plug, pic->width(), pic->height());
			
			//gtk_widget_realize (plug);

			gtk_widget_show_all(plug);
	
			gtk_widget_add_events(plug, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
				| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
			
			g_signal_connect(G_OBJECT(plug),"destroy",G_CALLBACK(tray_destroy),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"event",G_CALLBACK(tray_event),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"button-release-event",G_CALLBACK(tray_up),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"button-press-event",G_CALLBACK(tray_down),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"focus-in-event",G_CALLBACK(tray_focus_In),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"focus-out-event",G_CALLBACK(tray_focus_Out),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"enter-notify-event",G_CALLBACK(tray_enterleave),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"leave-notify-event",G_CALLBACK(tray_enterleave),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"popup-menu",G_CALLBACK(cb_menu),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"scroll-event",G_CALLBACK(cb_scroll),(gpointer)this);
			g_signal_connect(G_OBJECT(plug),"expose-event", G_CALLBACK(cb_expose), (gpointer)this);
			
			win = gtk_plug_get_id(GTK_PLUG(plug));
			
			X11_window_dock(win);
			//XTray_RequestDock(gdk_display, win);
			
			//gdk_window_set_back_pixmap(plug->window, (GdkPixmap *)GDK_PARENT_RELATIVE, FALSE);
			//XSetWindowBackgroundPixmap(gdk_display, win, ParentRelative);
			
			updateTooltip();
			refresh();
		}
		
	}
	else
	{
		if (plug)
		{
			gtk_widget_destroy(plug);
		}
	}
}

long gTrayIcon::screenX()
{
	unsigned int ret;

	if (!plug) return 0;
	XTray_getPosition(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),&ret,NULL);

	return ret;
}

long gTrayIcon::screenY()
{
	unsigned int ret;

	if (!plug) return 0;
	XTray_getPosition(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),NULL,&ret);

	return ret;
}

long gTrayIcon::width()
{
	unsigned int ret;

	XTray_getSize(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),&ret,NULL);
	return ret;
}

long gTrayIcon::height()
{
	unsigned int ret;

	XTray_getSize(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),NULL,&ret);
	return ret;
}

void gTrayIcon::exit()
{
	gTrayIcon *icon;
	
	while((icon = get(0)))
		delete icon;
}

bool gTrayIcon::hasSystemTray()
{
	return X11_get_system_tray() != None;
}

#else



gTrayIcon::gTrayIcon()
{
	stub("no-X11/gTrayIcon class");
}

gTrayIcon::~gTrayIcon()
{
	stub("no-X11/gTrayIcon class");
}


void gTrayIcon::refresh()
{
	stub("no-X11/gTrayIcon class");
}

void gTrayIcon::setPicture(gPicture *picture)
{
	stub("no-X11/gTrayIcon class");
}

char* gTrayIcon::toolTip()
{
	stub("no-X11/gTrayIcon class");

}

void gTrayIcon::setToolTip(char* vl)
{
	stub("no-X11/gTrayIcon class");

}

bool gTrayIcon::isVisible()
{
	stub("no-X11/gTrayIcon class");
}

void gTrayIcon::setVisible(bool vl)
{
	stub("no-X11/gTrayIcon class");
}

long gTrayIcon::screenX()
{
	stub("no-X11/gTrayIcon class");
}

long gTrayIcon::screenY()
{
	stub("no-X11/gTrayIcon class");
}

long gTrayIcon::width()
{
	stub("no-X11/gTrayIcon class");
}

long gTrayIcon::height()
{
	stub("no-X11/gTrayIcon class");
}

void gTrayIcon::exit()
{
	stub("no-X11/gTrayIcon class");
}

#endif

