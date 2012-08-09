/***************************************************************************

  gmouse.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __GMOUSE_CPP

#include "widgets.h"
#include "widgets_private.h"
#include "gmouse.h"

#ifndef GAMBAS_DIRECTFB
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#endif
#endif

int gMouse::_isValid = 0;
int gMouse::_x;
int gMouse::_y;
int gMouse::_button;
int gMouse::_state;
int gMouse::_delta;
int gMouse::_orientation;
int gMouse::_start_x;
int gMouse::_start_y;
GdkEvent *gMouse::_event;

void gMouse::move(int x, int y)
{
	GdkDisplay* dpy;
	GdkWindow*  win=gdk_get_default_root_window();
	#ifdef GAMBAS_DIRECTFB
	stub("DIRECTFB/gMouse::move");
	#else	
	#ifdef GDK_WINDOWING_X11
	dpy=gdk_display_get_default();
	XWarpPointer(GDK_DISPLAY_XDISPLAY(dpy),GDK_WINDOW_XID(win),GDK_WINDOW_XID(win),0, 0,0, 0,x, y);
	#else
	stub("no-X11/gMouse::move");
	#endif
	#endif
}

int gMouse::button()
{
	if (_isValid) 
		return _button;
	else
		return 0; 
}

bool gMouse::left()
{
	return _isValid ? (_button & 1) : false;
}

bool gMouse::right()
{
	return _isValid ? (_button & 2) : false;
}

bool gMouse::middle()
{
	return _isValid ? (_button & 4) : false;
}

bool gMouse::shift()
{
	return _isValid ? (_state & GDK_SHIFT_MASK) : false;
}

bool gMouse::control()
{
	return _isValid ? (_state & GDK_CONTROL_MASK) : false;
}

bool gMouse::alt()
{
	return _isValid ? (_state & GDK_MOD1_MASK) : false;
}

bool gMouse::meta()
{
	return _isValid ? (_state & GDK_MOD2_MASK) : false;
}

bool gMouse::normal()
{
	return _isValid ? (_state & 0xFF) : false;
}

int gMouse::x()
{
	return _isValid ? _x : -1;
}

int gMouse::y()
{
	return _isValid ? _y : -1;
}

void gMouse::getScreenPos(int *x, int *y)
{
	gdk_display_get_pointer(gdk_display_get_default(), NULL, x, y, NULL);
}

int gMouse::screenX()
{
	gint x;
	gdk_display_get_pointer(gdk_display_get_default(), NULL, &x, NULL, NULL);
	return x;
}

int gMouse::screenY()
{
	gint y;
	gdk_display_get_pointer(gdk_display_get_default(), NULL, NULL, &y, NULL);
	return y;
}

int gMouse::delta()
{
	return _isValid ? _delta : -1;
}

int gMouse::orientation()
{
	return _isValid ? _orientation : -1;
}

//"Private"

void gMouse::setWheel(int dt,int orn)
{
	_delta = dt;
	_orientation = orn;
}

void gMouse::setStart(int sx, int sy)
{
	_start_x = sx;
	_start_y = sy;
}

void gMouse::setMouse(int x, int y, int button, int state)
{
	_delta = 0;
	_orientation = 0;
	
	_x = x;
	_y = y;
	_state = state;

	switch(button)
	{
		case 1: _button = 1; break;
		case 2: _button = 4; break;
		case 3: _button = 2; break;
		default:
			_button = 0;
			if (_state & GDK_BUTTON1_MASK) _button += 1;
			if (_state & GDK_BUTTON2_MASK) _button += 4;
			if (_state & GDK_BUTTON3_MASK) _button += 2;
	}
}

void gMouse::setEvent(GdkEvent *event)
{
	_event = event;
}

double gMouse::getAxis(GdkAxisUse axis)
{
	double value;
	
	if (gdk_event_get_axis(_event, axis, &value))
		return value;
	else
		return 0.0;
}

int gMouse::getType()
{
	GdkDevice *device;
	
	switch(_event->type)
	{
		case GDK_BUTTON_PRESS: case GDK_2BUTTON_PRESS: case GDK_3BUTTON_PRESS: case GDK_BUTTON_RELEASE: 
			device = ((GdkEventButton *)_event)->device; 
			break;
			
		case GDK_SCROLL:
			device = ((GdkEventScroll *)_event)->device; 
			break;
		
		case GDK_MOTION_NOTIFY:
			device = ((GdkEventMotion *)_event)->device; 
			break;
		
		case GDK_PROXIMITY_IN: case GDK_PROXIMITY_OUT:
			device = ((GdkEventProximity *)_event)->device; 
			break;
		
		default:
			device = NULL;
	}
	
	if (!device)
		return POINTER_MOUSE;
	
	switch(gdk_device_get_source(device))
	{
		case GDK_SOURCE_PEN: return POINTER_PEN;
		case GDK_SOURCE_ERASER: return POINTER_ERASER;
		case GDK_SOURCE_CURSOR: return POINTER_CURSOR;
		default: return POINTER_MOUSE;
	}
}
