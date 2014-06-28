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

#include <unistd.h>
#include "gapplication.h"
#include "gmouse.h"
#include "gtrayicon.h"

#include "gb.form.trayicon.h"

int gTrayIcon::_visible_count = 0;

/*************************************************************************

gTrayIcon

**************************************************************************/

/*static gboolean cb_activate(GtkStatusIcon *plug, gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	//gApplication::updateLastEventTime(event);

	if (data->onDoubleClick)
		data->onDoubleClick(data);

	return false;
}*/


static gboolean cb_button_press(GtkStatusIcon *plug, GdkEventButton *event, gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	if (data->onMousePress)
	{
		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, (int)event->x_root, (int)event->y_root, event->button, event->state);
		if (event->type == GDK_BUTTON_PRESS)
			data->onMousePress(data);
		else if (event->type == GDK_2BUTTON_PRESS)
			data->onDoubleClick(data);
		gMouse::invalidate();
	}

	/*if (event->button == 3)
		if (data->onMenu)
			data->onMenu(data);*/

	return false;
}

static gboolean cb_button_release(GtkStatusIcon *plug, GdkEventButton *event, gTrayIcon *data)
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

static gboolean cb_menu(GtkStatusIcon *plug, guint button, guint activate_time, gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;
	
	if (data->onMenu)
		data->onMenu(data);
	
	return false;
}

/*static gboolean tray_focus_In(GtkWidget *widget,GdkEventFocus *event,gTrayIcon *data)
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
}*/

static gboolean cb_scroll(GtkStatusIcon *plug, GdkEventScroll *event, gTrayIcon *data)
{
	GdkScrollDirection dir;
	int dt = 0;
	int ort = 0;
	
	if (!gApplication::userEvents()) return false;
	if (gApplication::loopLevel() > data->loopLevel()) return false;

	if (data->onMouseWheel)
	{
		dir = event->direction;

#ifdef GTK3
		if (dir == GDK_SCROLL_SMOOTH)
			return false;
		/*{
			gdouble dx = 0, dy = 0;
			gdk_event_get_scroll_deltas((GdkEvent *)event, &dx, &dy);
			if (fabs(dy) > fabs(dx))
				dir = (dy < 0) ? GDK_SCROLL_UP : GDK_SCROLL_DOWN;
			else
				dir = (dx < 0) ? GDK_SCROLL_LEFT : GDK_SCROLL_RIGHT;
		}*/
#endif

		switch (dir)
		{
			case GDK_SCROLL_UP: dt=1; ort=1; break;
			case GDK_SCROLL_DOWN: dt=-1; ort=1; break;
			case GDK_SCROLL_LEFT: dt=-1; ort=0; break;
			case GDK_SCROLL_RIGHT: default: dt=1; ort=0; break;
		}

		gMouse::validate();
		gMouse::setMouse((int)event->x, (int)event->y, (int)event->x_root, (int)event->y_root, 0, event->state);
		gMouse::setWheel(dt, ort);
		data->onMouseWheel(data);
		gMouse::invalidate();
	}
	
	return false;
}


GList *gTrayIcon::trayicons = NULL;
gPicture *gTrayIcon::_default_icon = NULL;

gTrayIcon::gTrayIcon()
{
	plug = NULL;
	buftext = NULL;
	_icon = NULL;
	_loopLevel = 0;

	onMousePress = NULL;
	onMouseRelease = NULL;
	onMouseWheel = NULL;
	onMenu = NULL;
	onFocusEnter = NULL;
	onFocusLeave = NULL;
	onDoubleClick = NULL;
	onEnter = NULL;
	onLeave = NULL;
	
	trayicons = g_list_append(trayicons, (gpointer)this);
}

gTrayIcon::~gTrayIcon()
{
	setVisible(false);

	gPicture::assign(&_icon);
	
	if (buftext) 
	{
		g_free(buftext);
		buftext = NULL;
	}
	
	trayicons = g_list_remove(trayicons, (gpointer)this);
	
	if (!trayicons && _default_icon)
	{
		delete _default_icon;
		_default_icon = NULL;
	}

	if (onDestroy) (*onDestroy)(this);
}

gPicture *gTrayIcon::defaultIcon()
{
	if (!_default_icon)
	{
		GdkPixbuf *img = gdk_pixbuf_new_from_data(_default_trayicon_data, GDK_COLORSPACE_RGB, TRUE, 8,
																							DEFAULT_TRAYICON_WIDTH, DEFAULT_TRAYICON_HEIGHT,
																							DEFAULT_TRAYICON_WIDTH * sizeof(int), NULL, NULL);
		_default_icon = new gPicture(img);
	}
	
	return _default_icon;
}

void gTrayIcon::updatePicture()
{
	GdkPixbuf *pixbuf;

	if (!plug)
		return;

	if (_icon)
		pixbuf = _icon->getPixbuf();
	else
		pixbuf = defaultIcon()->getPixbuf();

	gtk_status_icon_set_from_pixbuf(plug, pixbuf);

	_iconw = gdk_pixbuf_get_width(pixbuf);
	_iconh = gdk_pixbuf_get_height(pixbuf);
}

void gTrayIcon::setPicture(gPicture *picture)
{
	gPicture::assign(&_icon, picture);
	updatePicture();
}

char* gTrayIcon::toolTip()
{
	return buftext;
}

void gTrayIcon::updateTooltip()
{
	if (!plug)
		return;

	gtk_status_icon_set_tooltip_text(plug, buftext);
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

static void hide_icon(GtkStatusIcon *plug)
{
	gtk_status_icon_set_visible(plug, FALSE);
	g_object_unref(plug);
}

void gTrayIcon::setVisible(bool vl)
{
	if (vl)
	{
		if (!plug)
		{
			_loopLevel = gApplication::loopLevel() + 1;
			
			plug = gtk_status_icon_new();

			updatePicture();
			updateTooltip();

			#ifdef GDK_WINDOWING_X11
			// Needed, otherwise the icon does not appear in Gnome or XFCE notification area!
			XSizeHints hints;
			hints.flags = PMinSize;
			hints.min_width = _iconw;
			hints.min_height = _iconh;
			XSetWMNormalHints(gdk_x11_display_get_xdisplay(gdk_display_get_default()), gtk_status_icon_get_x11_window_id(plug), &hints);
			#endif

			gtk_status_icon_set_visible(plug, TRUE);

			//g_signal_connect(G_OBJECT(plug), "destroy", G_CALLBACK(cb_destroy),(gpointer)this);
			g_signal_connect(G_OBJECT(plug), "button-press-event", G_CALLBACK(cb_button_press), (gpointer)this);
			g_signal_connect(G_OBJECT(plug), "button-release-event", G_CALLBACK(cb_button_release),(gpointer)this);
			//g_signal_connect(G_OBJECT(plug), "activate", G_CALLBACK(cb_activate),(gpointer)this);
			//g_signal_connect(G_OBJECT(plug),"focus-in-event",G_CALLBACK(tray_focus_In),(gpointer)this);
			//g_signal_connect(G_OBJECT(plug),"focus-out-event",G_CALLBACK(tray_focus_Out),(gpointer)this);
			//g_signal_connect(G_OBJECT(plug),"enter-notify-event",G_CALLBACK(tray_enterleave),(gpointer)this);
			//g_signal_connect(G_OBJECT(plug),"leave-notify-event",G_CALLBACK(tray_enterleave),(gpointer)this);
			g_signal_connect(G_OBJECT(plug), "popup-menu", G_CALLBACK(cb_menu), (gpointer)this);
			g_signal_connect(G_OBJECT(plug), "scroll-event", G_CALLBACK(cb_scroll), (gpointer)this);
			//g_signal_connect(G_OBJECT(plug),"expose-event", G_CALLBACK(cb_expose), (gpointer)this);
			
			_visible_count++;

			usleep(10000); // BUG: Embedding too fast sometimes fails with GTK+
		}
	}
	else
	{
		if (plug)
		{
			GB.Post((void (*)())hide_icon, (intptr_t)plug);
			plug = NULL;
			_visible_count--;
		}
	}
}

int gTrayIcon::screenX()
{
	GdkRectangle area;

	if (!plug || !gtk_status_icon_get_geometry(plug, NULL, &area, NULL))
		return 0;

	return area.x;
}

int gTrayIcon::screenY()
{
	GdkRectangle area;

	if (!plug || !gtk_status_icon_get_geometry(plug, NULL, &area, NULL))
		return 0;

	return area.y;
}

int gTrayIcon::width()
{
	GdkRectangle area;

	if (!plug || !gtk_status_icon_get_geometry(plug, NULL, &area, NULL))
		return 0;

	return area.width;
}

int gTrayIcon::height()
{
	GdkRectangle area;

	if (!plug || !gtk_status_icon_get_geometry(plug, NULL, &area, NULL))
		return 0;

	return area.height;
}

void gTrayIcon::exit()
{
	gTrayIcon *icon;
	
	while((icon = get(0)))
		delete icon;
}

bool gTrayIcon::hasSystemTray()
{
	return true;
}
