/***************************************************************************

  gmouse.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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
#include "gmouse.h"

int gMouse::_isValid = 0;
int gMouse::_x;
int gMouse::_y;
int gMouse::_screen_x;
int gMouse::_screen_y;
int gMouse::_button;
int gMouse::_state;
int gMouse::_delta;
int gMouse::_orientation;
int gMouse::_start_x;
int gMouse::_start_y;
int gMouse::_dx = 0;
int gMouse::_dy = 0;
GdkEvent *gMouse::_event = 0;

#ifdef GTK3
static GdkDevice *get_pointer()
{
	return gdk_device_manager_get_client_pointer(gdk_display_get_device_manager(gdk_display_get_default()));
}
#endif

void gMouse::move(int x, int y)
{
	GdkDisplay* dpy = gdk_display_get_default();
#ifdef GTK3
	gdk_device_warp(get_pointer(), gdk_display_get_default_screen(dpy), x, y);
#else
	gdk_display_warp_pointer(dpy, gdk_display_get_default_screen(dpy), x, y);
#endif
}

int gMouse::button()
{
	return _isValid ? _button : 0;
}

int gMouse::state()
{
	return _isValid ? _state : 0;
}

bool gMouse::left()
{
	return _isValid ? (_state & GDK_BUTTON1_MASK || _button == 1) : false;
}

bool gMouse::right()
{
	return _isValid ? (_state & GDK_BUTTON3_MASK || _button == 3) : false;
}

bool gMouse::middle()
{
	return _isValid ? (_state & GDK_BUTTON2_MASK || _button == 2) : false;
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
	return _isValid ? _x + _dx : -1;
}

int gMouse::y()
{
	return _isValid ? _y + _dy : -1;
}

void gMouse::getScreenPos(int *x, int *y)
{
	if (_isValid)
	{
		*x = _screen_x;
		*y = _screen_y;
	}
	else
	{
#ifdef GTK3
		gdk_device_get_position(get_pointer(), NULL, x, y);
#else
		gdk_display_get_pointer(gdk_display_get_default(), NULL, x, y, NULL);
#endif
	}
}

int gMouse::screenX()
{
	gint x;
	
	if (_isValid)
		x = _screen_x;
	else
#ifdef GTK3
		gdk_device_get_position(get_pointer(), NULL, &x, NULL);
#else
		gdk_display_get_pointer(gdk_display_get_default(), NULL, &x, NULL, NULL);
#endif
	
	return x;
}

int gMouse::screenY()
{
	gint y;
	
	if (_isValid)
		y = _screen_y;
	else
#ifdef GTK3
		gdk_device_get_position(get_pointer(), NULL, NULL, &y);
#else
		gdk_display_get_pointer(gdk_display_get_default(), NULL, NULL, &y, NULL);
#endif
	
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

void gMouse::setMouse(int x, int y, int sx, int sy, int button, int state)
{
	_delta = 0;
	_orientation = 0;
	
	_x = x;
	_y = y;
	_state = state;
	_button = button;
	_screen_x = sx;
	_screen_y = sy;

	/*switch(button)
	{
		case 1: _button = 1; break;
		case 2: _button = 4; break;
		case 3: _button = 2; break;
		default:
			_button = 0;
			if (_state & GDK_BUTTON1_MASK) _button += 1;
			if (_state & GDK_BUTTON2_MASK) _button += 4;
			if (_state & GDK_BUTTON3_MASK) _button += 2;
	}*/
}

static GdkDevice *get_event_device(GdkEvent *event)
{
	switch(event->type)
	{
		case GDK_BUTTON_PRESS: case GDK_2BUTTON_PRESS: case GDK_3BUTTON_PRESS: case GDK_BUTTON_RELEASE: 
			return ((GdkEventButton *)event)->device; 
			
		case GDK_SCROLL:
			return ((GdkEventScroll *)event)->device; 
		
		case GDK_MOTION_NOTIFY:
			return ((GdkEventMotion *)event)->device; 
		
		case GDK_PROXIMITY_IN: case GDK_PROXIMITY_OUT:
			return ((GdkEventProximity *)event)->device; 
		
		default:
			return NULL;
	}
}

void gMouse::setEvent(GdkEvent *event)
{
	_event = gdk_event_copy(event);
	//fprintf(stderr, "device = %p\n", get_event_device(event));
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
	GdkDevice *device = get_event_device(_event);
	
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

void gMouse::initDevices()
{
#ifndef GTK3
	static bool done = false;
	
	GList *devices;
	GdkDevice *device;
	
	if (done)
		return;
	
	//fprintf(stderr, "initDevices\n");
	
	devices = gdk_devices_list();
	
	while (devices)
	{
		device = (GdkDevice *)devices->data;
		
		if (gdk_device_get_source(device) != GDK_SOURCE_MOUSE)
		{
			//fprintf(stderr, "enable device '%s'\n", gdk_device_get_name(device));
			gdk_device_set_mode(device, GDK_MODE_SCREEN);
		}
		
		devices = devices->next;
	}
	
	done = true;
#endif
}

double gMouse::getPointerX()
{
	return ((GdkEventMotion *)_event)->x + _dx;
}

double gMouse::getPointerY()
{
	return ((GdkEventMotion *)_event)->y + _dy;
}

double gMouse::getPointerScreenX()
{
	return getAxis(GDK_AXIS_X);
}

double gMouse::getPointerScreenY()
{
	return getAxis(GDK_AXIS_Y);
}

void gMouse::invalidate()
{
	_isValid--;
	
	if (_isValid == 0)
	{
		if (_event)
		{
			gdk_event_free(_event);
			_event = 0;
		}
	}
}

void gMouse::translate(int dx, int dy)
{
	_dx = dx;
	_dy = dy;
}
