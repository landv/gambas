/***************************************************************************

  x11.c

  Common X11 routines for gb.qt and gb.gtk

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __X11_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "x11.h"

#define MAX_WINDOW_PROP 16

PUBLIC Atom X11_atom_net_wm_state;
PUBLIC Atom X11_atom_net_wm_state_above;
PUBLIC Atom X11_atom_net_wm_state_below;
PUBLIC Atom X11_atom_net_wm_state_stays_on_top;
PUBLIC Atom X11_atom_net_wm_state_skip_taskbar;
PUBLIC Atom X11_atom_net_wm_window_type;
PUBLIC Atom X11_atom_net_wm_window_type_normal;
PUBLIC Atom X11_atom_net_wm_window_type_utility;
PUBLIC Atom X11_atom_net_wm_desktop;
PUBLIC Atom X11_atom_net_current_desktop;

static bool _init = FALSE;

static Display *_display = NULL;
static Window _root = 0;

static bool _atom_init = FALSE;

typedef
	struct {
		int count;
		Atom atoms[MAX_WINDOW_PROP];
		}
	X11_PROPERTY;

static X11_PROPERTY _window_prop;
static X11_PROPERTY _window_save[2];

static void init_atoms()
{
  if (_atom_init)
    return;

	fprintf(stderr, "init_atom: display = %p\n", _display);
	X11_atom_net_current_desktop = XInternAtom(_display, "_NET_CURRENT_DESKTOP", True);
	fprintf(stderr, "init_atom: #1\n");
  X11_atom_net_wm_state = XInternAtom(_display, "_NET_WM_STATE", True);
  X11_atom_net_wm_state_above = XInternAtom(_display, "_NET_WM_STATE_ABOVE", True);
  X11_atom_net_wm_state_below = XInternAtom(_display, "_NET_WM_STATE_BELOW", True);
  X11_atom_net_wm_state_stays_on_top = XInternAtom(_display, "_NET_WM_STATE_STAYS_ON_TOP", True);
  X11_atom_net_wm_state_skip_taskbar = XInternAtom(_display, "_NET_WM_STATE_SKIP_TASKBAR", True);

	X11_atom_net_wm_desktop = XInternAtom(_display, "_NET_WM_DESKTOP", True);
	X11_atom_net_wm_window_type = XInternAtom(_display, "_NET_WM_WINDOW_TYPE", True);
	X11_atom_net_wm_window_type_normal = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_NORMAL", True);
	X11_atom_net_wm_window_type_utility = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_UTILITY", True);

  _atom_init = TRUE;
}

static void get_property(Window wid, Atom prop, int maxlength, unsigned char **data, unsigned long *count)
{
  Atom type;
  int format;
  unsigned long after;

  XGetWindowProperty(_display, wid, prop, 0, maxlength / 4,
    False, AnyPropertyType, &type, &format,
    count, &after, data);
}

static void load_window_state(Window win, Atom prop)
{
  unsigned long length = 0;
  unsigned char *data = NULL;

  _window_prop.count = 0;

	//get_property(win, X11_atom_net_wm_state, MAX_WINDOW_STATE * sizeof(Atom), &data, &length);
	get_property(win, prop, MAX_WINDOW_PROP * sizeof(Atom), &data, &length);

  if (length > MAX_WINDOW_PROP)
    length = MAX_WINDOW_PROP;

  _window_prop.count = length;
  memcpy(_window_prop.atoms, data, length * sizeof(Atom));

  XFree(data);
}

static void save_window_state(Window win, Atom prop)
{
	if (_window_prop.count > 0)
	{
  	XChangeProperty(_display, win, prop, XA_ATOM, 32, PropModeReplace,
      	(unsigned char *)_window_prop.atoms, _window_prop.count);
	}
}

static bool has_window_state(Atom prop)
{
  int i;

  for (i = 0; i < _window_prop.count; i++)
  {
    if (_window_prop.atoms[i] == prop)
      return TRUE;
  }

  return FALSE;
}

static void set_window_state(Atom prop)
{
  if (has_window_state(prop))
    return;

  if (_window_prop.count == MAX_WINDOW_PROP)
  {
    fprintf(stderr, "X11: set_window_state: Too many properties in window\n");
    return;
  }

  _window_prop.atoms[_window_prop.count++] = prop;
}

static void clear_window_state(Atom prop)
{
  int i;

  for (i = 0; i < _window_prop.count; i++)
  {
    if (_window_prop.atoms[i] == prop)
    {
      _window_prop.count--;

      for (; i < _window_prop.count; i++)
        _window_prop.atoms[i] = _window_prop.atoms[i + 1];

      return;
    }
  }
}


PUBLIC bool X11_init()
{
	if (_init)
		return FALSE;
		
	GB.GetComponentInfo("DISPLAY", POINTER(&_display));
	GB.GetComponentInfo("ROOT_WINDOW", POINTER(&_root));
  
  /*_display = XOpenDisplay(NULL);
  fprintf(stderr, "_display = %p\n", _display);
  _root = DefaultRootWindow(_display);*/
  
  _init = _display != NULL;
  
  if (_init)
  	init_atoms();
  else
  	fprintf(stderr, "WARNING: X11_init() has failed\n");
  
  return !_init;
}

PUBLIC void X11_exit()
{
	/*if (_init)
	{
		XCloseDisplay(_display);
	}*/
}


PUBLIC void X11_window_change_property(Window window, bool visible, Atom property, bool set)
{
  XEvent e;
  long mask = (SubstructureRedirectMask | SubstructureNotifyMask);

  if (visible)
  {
    e.xclient.type = ClientMessage;
    e.xclient.message_type = X11_atom_net_wm_state;
    e.xclient.display = _display;
    e.xclient.window = window;
    e.xclient.format = 32;
    e.xclient.data.l[0] = set ? 1 : 0;
    e.xclient.data.l[1] = property;
    e.xclient.data.l[2] = 0;
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;

    XSendEvent(_display, _root, False, mask, &e);
  }
  else
  {
    load_window_state(window, X11_atom_net_wm_state);

    if (set)
      set_window_state(property);
    else
      clear_window_state(property);

    save_window_state(window, X11_atom_net_wm_state);
  }
}


PUBLIC bool X11_window_has_property(Window window, Atom property)
{
  load_window_state(window, X11_atom_net_wm_state);
  return has_window_state(property);
}

PUBLIC void X11_sync(void)
{
  XSync(_display, False);
}

PUBLIC void X11_window_save_properties(Window window)
{
  load_window_state(window, X11_atom_net_wm_state);
  _window_save[0] = _window_prop;
  //load_window_state(window, X11_atom_net_wm_window_type);
  //_window_save[1] = _window_prop;
}

PUBLIC void X11_window_restore_properties(Window window)
{
  _window_prop = _window_save[0];
  save_window_state(window, X11_atom_net_wm_state);
  //_window_prop = _window_save[1];
  //save_window_state(window, X11_atom_net_wm_window_type);
}

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define OPCODE "_NET_SYSTEM_TRAY_OPCODE"

PUBLIC void X11_window_dock(Window window)
{
  Window xmanager=None;
  XClientMessageEvent ev;
  Atom OpCodeAtom;
  Screen *xscreen;
  char buf[256];
  Atom selection_atom;

  buf[0]=0;

  xscreen = DefaultScreenOfDisplay(_display);
  sprintf(buf,"_NET_SYSTEM_TRAY_S%d",XScreenNumberOfScreen(xscreen));
  selection_atom = XInternAtom(_display, buf, 0);

  XGrabServer(_display);

  xmanager = XGetSelectionOwner(_display, selection_atom);
  if (xmanager != None)
    XSelectInput(_display, xmanager, StructureNotifyMask);

  XUngrabServer(_display);
  XFlush(_display);

  /***********************************************
    Dock Tray Icon
  ************************************************/

  OpCodeAtom = XInternAtom(_display, OPCODE, 0);

  ev.type = ClientMessage;
  ev.window = xmanager;
  ev.message_type = OpCodeAtom;
  ev.format = 32;
  ev.data.l[0] = 0;
  ev.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
  ev.data.l[2] = window;
  ev.data.l[3] = 0;
  ev.data.l[4] = 0;

  XSendEvent(_display, xmanager, 0, NoEventMask, (XEvent *)&ev);
  XSync(_display, 0);
}

PUBLIC void X11_window_startup(Window window, int x, int y, int w, int h)
{
  XSizeHints s;

  s.flags = USPosition | PPosition | USSize | PSize;

  s.x = x;
  s.y = y;
  s.width = w;
  s.height = h;

  XSetWMNormalHints(_display, window, &s);
}

// ### Do not forget to call XFree() on window_list once finished with it

PUBLIC void X11_find_windows(Window **window_list, int *count)
{
	static Atom _net_client_list = 0;

	if (!_net_client_list)
		_net_client_list = XInternAtom(_display, "_NET_CLIENT_LIST", True);

	get_property(_root, _net_client_list, 1024 * sizeof(Window), (unsigned char **)window_list, (unsigned long *)count);
}

// ### Do not forget to call XFree() on result once finished with it

PUBLIC void X11_get_window_title(Window window, char **result, long *length)
{
	get_property(window, XA_WM_NAME, 256, (unsigned char **)result, (unsigned long *)length);
}

PUBLIC void X11_get_window_class(Window window, char **result, long *length)
{
	get_property(window, XA_WM_CLASS, 256, (unsigned char **)result, (unsigned long *)length);
}

PUBLIC void X11_get_window_role(Window window, char **result, long *length)
{
	static Atom wm_window_role = (Atom)0;

	if (!wm_window_role)
		wm_window_role = XInternAtom(_display, "WM_WINDOW_ROLE", True);

	get_property(window, wm_window_role, 256, (unsigned char **)result, (unsigned long *)length);
}


// Makes a tool window

PUBLIC void X11_set_window_tool(Window window, int tool, Window parent)
{
	load_window_state(window, X11_atom_net_wm_window_type);

	if (tool)
	{
		set_window_state(X11_atom_net_wm_window_type_utility);
		clear_window_state(X11_atom_net_wm_window_type_normal);
		if (parent)
			XSetTransientForHint(_display, window, parent);
	}
	else
	{
		clear_window_state(X11_atom_net_wm_window_type_utility);
		set_window_state(X11_atom_net_wm_window_type_normal);
	}

	save_window_state(window, X11_atom_net_wm_window_type);
}

PUBLIC int X11_get_window_tool(Window window)
{
	load_window_state(window, X11_atom_net_wm_window_type);
  return has_window_state(X11_atom_net_wm_window_type_utility);
}


// Set window desktop

PUBLIC void X11_window_set_desktop(Window window, bool visible, int desktop)
{
  XEvent e;
  long mask = (SubstructureRedirectMask | SubstructureNotifyMask);

  if (visible)
  {
    e.xclient.type = ClientMessage;
    e.xclient.message_type = X11_atom_net_wm_desktop;
    e.xclient.display = _display;
    e.xclient.window = window;
    e.xclient.format = 32;
    e.xclient.data.l[0] = desktop;
    e.xclient.data.l[1] = 1;
    e.xclient.data.l[2] = 0;
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;

    XSendEvent(_display, _root, False, mask, &e);
  }
  else
  {
  	XChangeProperty(_display, window, X11_atom_net_wm_desktop, XA_CARDINAL, 32, PropModeReplace,
      	(unsigned char *)&desktop, 1);
  }
}


PUBLIC long X11_window_get_desktop(Window window)
{
	unsigned long length = 0;
	unsigned char *data = NULL;
	long desktop = 0;

	get_property(window, X11_atom_net_wm_desktop, sizeof(int), &data, &length);

	if (data)
	{
		desktop = *((long *)data);
		XFree(data);
	}
	
	return desktop;
}

PUBLIC long X11_get_current_desktop()
{
	unsigned long length = 0;
	unsigned char *data = NULL;
	long desktop;

	get_property(_root, X11_atom_net_current_desktop, sizeof(int), &data, &length);

	desktop = *((long *)data);

	XFree(data);
	
	return desktop;
}
