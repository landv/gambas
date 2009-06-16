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

#include "x11.h"

#define MAX_WINDOW_PROP 16

Atom X11_atom_net_wm_state;
Atom X11_atom_net_wm_state_above;
Atom X11_atom_net_wm_state_below;
Atom X11_atom_net_wm_state_stays_on_top;
Atom X11_atom_net_wm_state_skip_taskbar;
Atom X11_atom_net_wm_window_type;
Atom X11_atom_net_wm_window_type_normal;
Atom X11_atom_net_wm_window_type_utility;
Atom X11_atom_net_wm_desktop;
Atom X11_atom_net_current_desktop;

Atom X11_atom_motif_wm_hints = None;

static Display *_display;
static Window _root;

static bool _atom_init = FALSE;

typedef
	struct {
		char *name;
		Atom atom;
		}
	X11_ATOM;

typedef
	struct {
		int count;
		Atom atoms[MAX_WINDOW_PROP];
		}
	X11_PROPERTY;

static X11_PROPERTY _window_prop;
static X11_PROPERTY _window_save[2];

static X11_ATOM _atoms[] =
{
	{"_NET_WM_WINDOW_TYPE_NORMAL"},
	{"_NET_WM_WINDOW_TYPE_DESKTOP"},
	{"_NET_WM_WINDOW_TYPE_DOCK"},
	{"_NET_WM_WINDOW_TYPE_TOOLBAR"},
	{"_NET_WM_WINDOW_TYPE_MENU"},
	{"_NET_WM_WINDOW_TYPE_UTILITY"},
	{"_NET_WM_WINDOW_TYPE_SPLASH"},
	{"_NET_WM_WINDOW_TYPE_DIALOG"},
	{"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU"},
	{"_NET_WM_WINDOW_TYPE_POPUP_MENU"},
	{"_NET_WM_WINDOW_TYPE_TOOLTIP"},
	{"_NET_WM_WINDOW_TYPE_NOTIFICATION"},
	{"_NET_WM_WINDOW_TYPE_COMBO"},
	{"_NET_WM_WINDOW_TYPE_DND"},
	{NULL}
};

typedef 
	struct {
		unsigned long flags;
		unsigned long functions;
		unsigned long decorations;
		long input_mode;
		unsigned long status;
	} 
	MwmHints;

static void init_atoms()
{
  if (_atom_init)
    return;

	X11_atom_net_current_desktop = XInternAtom(_display, "_NET_CURRENT_DESKTOP", True);

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

static Atom get_atom(int index)
{
	X11_ATOM *a = &_atoms[index];
	
	if (!a->atom)
		a->atom = XInternAtom(_display, a->name, True);
		
	return a->atom;
}

static int find_atom(Atom atom)
{
	int i = 0;
	X11_ATOM *p = _atoms;
	
	while (p->name)
	{
		if (!p->atom)
			p->atom = XInternAtom(_display, p->name, True);
		if (p->atom == atom)
			return i;
		p++;
		i++;
	}
	return (-1);
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


void X11_init(Display *display, Window root)
{
  _display = display;
  _root = root;
  init_atoms();
}

void X11_exit()
{
}


void X11_window_change_property(Window window, bool visible, Atom property, bool set)
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


bool X11_window_has_property(Window window, Atom property)
{
  load_window_state(window, X11_atom_net_wm_state);
  return has_window_state(property);
}

void X11_sync(void)
{
  XSync(_display, False);
}

void X11_window_save_properties(Window window)
{
  load_window_state(window, X11_atom_net_wm_state);
  _window_save[0] = _window_prop;
  //load_window_state(window, X11_atom_net_wm_window_type);
  //_window_save[1] = _window_prop;
}

void X11_window_restore_properties(Window window)
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

void X11_window_dock(Window window)
{
  Window xmanager=None;
  XClientMessageEvent ev;
  Atom OpCodeAtom;
  Screen *xscreen;
  char buf[256];
  Atom selection_atom;

  xscreen = DefaultScreenOfDisplay(_display);
  sprintf(buf, "_NET_SYSTEM_TRAY_S%d", XScreenNumberOfScreen(xscreen));
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

void X11_window_startup(Window window, int x, int y, int w, int h)
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

void X11_find_windows(Window **window_list, int *count)
{
	static Atom _net_client_list = 0;

	if (!_net_client_list)
		_net_client_list = XInternAtom(_display, "_NET_CLIENT_LIST", True);

	get_property(_root, _net_client_list, 1024 * sizeof(Window), (unsigned char **)window_list, (unsigned long *)count);
}

// ### Do not forget to call XFree() on result once finished with it

void X11_get_window_title(Window window, char **result, int *length)
{
	unsigned long l;
	get_property(window, XA_WM_NAME, 256, (unsigned char **)result, &l);
	*length = (int)l;
}

void X11_get_window_class(Window window, char **result, int *length)
{
	unsigned long l;
	get_property(window, XA_WM_CLASS, 256, (unsigned char **)result, &l);
	*length = (int)l;
}

void X11_get_window_role(Window window, char **result, int *length)
{
	static Atom wm_window_role = (Atom)0;
	unsigned long l;

	if (!wm_window_role)
		wm_window_role = XInternAtom(_display, "WM_WINDOW_ROLE", True);

	get_property(window, wm_window_role, 256, (unsigned char **)result, &l);
	*length = (int)l;
}


// Makes a tool window

void X11_set_window_tool(Window window, int tool, Window parent)
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

int X11_get_window_tool(Window window)
{
	load_window_state(window, X11_atom_net_wm_window_type);
  return has_window_state(X11_atom_net_wm_window_type_utility);
}


// Set window desktop

void X11_window_set_desktop(Window window, bool visible, int desktop)
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


int X11_window_get_desktop(Window window)
{
	unsigned long length = 0;
	unsigned char *data = NULL;
	int desktop = 0;

	get_property(window, X11_atom_net_wm_desktop, sizeof(int), &data, &length);

	if (data)
	{
		desktop = *((int *)data);
		XFree(data);
	}
	
	return desktop;
}

int X11_get_current_desktop()
{
	unsigned long length = 0;
	unsigned char *data = NULL;
	int desktop;

	get_property(_root, X11_atom_net_current_desktop, sizeof(int), &data, &length);

	desktop = *((int *)data);

	XFree(data);
	
	return desktop;
}

int X11_get_window_type(Window window)
{
	int index;
	
	load_window_state(window, X11_atom_net_wm_window_type);
	index = find_atom(_window_prop.atoms[0]);
	if (index < 0)
		return _NET_WM_WINDOW_TYPE_NORMAL;
	else
		return index;
}

void X11_set_window_type(Window window, int type)
{
	_window_prop.count = 1;
	_window_prop.atoms[0] = get_atom(type);
	save_window_state(window, X11_atom_net_wm_window_type);
}

void X11_set_transient_for(Window window, Window parent)
{
	XSetTransientForHint(_display, window, parent);
}

void X11_set_window_decorated(Window window, bool decorated)
{
	// Motif structures
	
	MwmHints *hints;
	MwmHints new_hints;

	uchar *data;
	Atom type;
	int format;
	ulong nitems;
	ulong bytes_after;
	
	if (X11_atom_motif_wm_hints == None)
		X11_atom_motif_wm_hints = XInternAtom(_display, "_MOTIF_WM_HINTS", True);

	XGetWindowProperty (_display, window,
		X11_atom_motif_wm_hints, 0, sizeof(MwmHints)/sizeof(long),
		False, AnyPropertyType, &type, &format, &nitems,
		&bytes_after, &data);
	
	if (type == None)
	{
		hints = &new_hints;
		hints->flags = 0;
		hints->functions = 0;
		hints->input_mode = 0;
		hints->status = 0;
		hints->decorations = 0;
	}
	else
		hints = (MwmHints *)data;
	
	hints->flags |= (1 << 1);
	hints->decorations = decorated ? 1 : 0;	
	
	XChangeProperty(_display, window,
		X11_atom_motif_wm_hints, X11_atom_motif_wm_hints, 32, PropModeReplace,
		(uchar *)hints, sizeof (MwmHints)/sizeof(long));
	
	if (hints != &new_hints)
		XFree (hints);
}

void X11_window_remap(Window window)
{
	XWithdrawWindow(_display, window, DefaultScreen(_display));
	XUnmapWindow(_display, window);
	XMapWindow(_display, window);
}

void X11_window_activate(Window window)
{
	XSetInputFocus(_display, window, RevertToParent, CurrentTime);
}
