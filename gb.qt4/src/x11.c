/***************************************************************************

  x11.c

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

#define __X11_C

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "x11.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const GB_INTERFACE *GB_PTR;
#define GB (*GB_PTR)

#ifdef __cplusplus
}

#endif


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
Atom X11_atom_net_workarea = None;

Atom X11_atom_motif_wm_hints = None;
Atom X11_atom_net_system_tray = None;

Atom X11_atom_net_supported;
Atom *_supported = NULL;

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

static char *_property_value = NULL;

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
	X11_atom_net_supported = XInternAtom(_display, "_NET_SUPPORTED", True);
	
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

#define PROPERTY_START_READ 1024
#define PROPERTY_NEXT_READ 1024

#if 0
	// On 64 bits OS, format 32 are actually long, i.e. int padded to 64 bits!
	#if OS_64BITS
	if (format == 32)
	{
		int *p = (int *)_property_value;
		for (i = 0; i < count; i++)
			p[i] = *((long *)data + i);
	}
	else
	#endif
	{
		memcpy(_property_value, (char *)data, count * size);
	}
#endif

char *X11_get_property(Window wid, Atom prop, Atom *type, int *format, int *pcount)
{
	uchar *data;
  unsigned long count;
  unsigned long after;
	unsigned long offset;
	int size, offset_size;

	*pcount = 0;

	if (XGetWindowProperty(_display, wid, prop, 0, PROPERTY_START_READ / sizeof(int32_t),
			False, AnyPropertyType, type, format,
			&count, &after, &data) != Success)
		return NULL;
	
	*pcount += count;
	
	size = *format == 32 ? sizeof(long) : ( *format == 16 ? sizeof(short) : 1 );
	offset_size = *format == 32 ? sizeof(int32_t) : ( *format == 16 ? sizeof(short) : 1 );
	
	//fprintf(stderr, "X11_get_property: format = %d size = %d count = %ld after = %ld\n", *format, size, count, after);
	
	GB.FreeString(&_property_value);
	_property_value = GB.NewString((char *)data, count * size);
	XFree(data);
	
	offset = count * offset_size / sizeof(int32_t);
	
	while (after)
	{
		//fprintf(stderr, "X11_get_property: offset = %ld read = %ld\n", offset, Min(after, PROPERTY_NEXT_READ) / sizeof(int32_t));
	
		if (XGetWindowProperty(_display, wid, prop, offset, Min(after, PROPERTY_NEXT_READ) / sizeof(int32_t),
				False, AnyPropertyType, type, format,
				&count, &after, &data) != Success)
			return NULL;

		*pcount += count;
		offset += count * offset_size / sizeof(int32_t);
		
		//fprintf(stderr, "X11_get_property: format = %d size = %d count = %ld after = %ld next offset = %ld\n", *format, size, count, after, offset);
	
		_property_value = GB.AddString(_property_value, (char *)data, count * size);
		XFree(data);
	}
	
	return _property_value;
}

static char *get_property(Window wid, Atom prop, int *count)
{
	Atom type;
	int format;
	
	return X11_get_property(wid, prop, &type, &format, count);
}

static void load_window_state(Window win, Atom prop)
{
	int length;
	char *data;

	_window_prop.count = 0;

	//get_property(win, X11_atom_net_wm_state, MAX_WINDOW_STATE * sizeof(Atom), &data, &length);
	data = get_property(win, prop, &length);

	if (length > MAX_WINDOW_PROP)
		length = MAX_WINDOW_PROP;

	_window_prop.count = length;
	memcpy(_window_prop.atoms, data, length * sizeof(Atom));
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


static void init_net_supported()
{
	char *data;
	int count;

	if (_supported)
		GB.FreeArray(&_supported);

	data = get_property(_root, X11_atom_net_supported, &count);
	if (!data)
		return;
	
	GB.NewArray(&_supported, sizeof(Atom), count);
	memcpy(_supported, data, sizeof(Atom) * count);
}


void X11_init(Display *display, Window root)
{
	_display = display;
	_root = root;
	init_atoms();
	init_net_supported();
}

void X11_exit()
{
	if (_supported)
		GB.FreeArray(&_supported);
	if (_property_value)
		GB.FreeString(&_property_value);
}


void X11_window_change_property(Window window, bool visible, Atom property, bool set)
{
	XEvent e;

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

		XSendEvent(_display, _root, False, (SubstructureRedirectMask | SubstructureNotifyMask), &e);
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

	*window_list = (Window *)get_property(_root, _net_client_list, count);
}

// ### Do not forget to call XFree() on result once finished with it

void X11_get_window_title(Window window, char **result, int *length)
{
	int l;
	*result = get_property(window, XA_WM_NAME, &l);
	*length = l;
}

void X11_get_window_class(Window window, char **result, int *length)
{
	int l;
	*result = get_property(window, XA_WM_CLASS, &l);
	*length = l;
}

void X11_get_window_role(Window window, char **result, int *length)
{
	static Atom wm_window_role = (Atom)0;
	int l;

	if (!wm_window_role)
		wm_window_role = XInternAtom(_display, "WM_WINDOW_ROLE", True);

	*result = get_property(window, wm_window_role, &l);
	*length = l;
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
	int length = 0;
	char *data = NULL;
	int desktop = 0;

	data = get_property(window, X11_atom_net_wm_desktop, &length);

	if (data)
		desktop = *((int *)data);
	
	return desktop;
}

int X11_get_current_desktop()
{
	int length = 0;
	char *data;
	int desktop;

	data = get_property(_root, X11_atom_net_current_desktop, &length);

	desktop = *((int *)data);
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

bool X11_get_available_geometry(int screen, int *x, int *y, int *w, int *h)
{
	if (X11_atom_net_workarea == None)
		X11_atom_net_workarea = XInternAtom(_display, "_NET_WORKAREA", True);

	Atom ret;
	int format, e;
	unsigned char *data = NULL;
	unsigned long nitems, after;
	bool err = TRUE;

	e = XGetWindowProperty(_display, RootWindow(_display, screen),
													X11_atom_net_workarea, 0, 4, False, XA_CARDINAL,
													&ret, &format, &nitems, &after, &data);

	if (e == Success && ret == XA_CARDINAL && format == 32 && nitems == 4) 
	{
		long *workarea = (long *)data;
		*x = workarea[0];
		*y = workarea[1];
		*w = workarea[2];
		*h = workarea[3];
		err = FALSE;
	} 

	if (data)
		XFree(data);
	
	return err;
}

bool X11_is_supported_by_WM(Atom atom)
{
	int i;
	
	if (_supported)
	{
		for (i = 0; i < GB.Count(_supported); i++)
		{
			if (_supported[i] == atom)
				return TRUE;
		}
	}

	return FALSE;
}

bool X11_send_move_resize_event(Window window, int x, int y, int w, int h)
{
 	static Atom _net_moveresize_window = None;
	XEvent e;

	if (!_net_moveresize_window)
		_net_moveresize_window = XInternAtom(_display, "_NET_MOVERESIZE_WINDOW", True);
	
	if (!X11_is_supported_by_WM(_net_moveresize_window))
		return TRUE;
	
	e.xclient.type = ClientMessage;
	e.xclient.message_type = _net_moveresize_window;
	e.xclient.display = _display;
	e.xclient.window = window;
	e.xclient.format = 32;
	e.xclient.data.l[0] = StaticGravity | 1<<8 | 1<<9 | 1<<10 | 1<<11 | 1<<12;
	e.xclient.data.l[1] = x;
	e.xclient.data.l[2] = y;
	e.xclient.data.l[3] = w;
	e.xclient.data.l[4] = h;
	XSendEvent(_display, _root, FALSE, (SubstructureNotifyMask | SubstructureRedirectMask), &e);
 
	return FALSE;
}

static Atom get_net_system_tray(void)
{
	char buf[64];
	
	if (X11_atom_net_system_tray == None)
	{
		sprintf(buf, "_NET_SYSTEM_TRAY_S%d", XScreenNumberOfScreen(DefaultScreenOfDisplay(_display)));
		X11_atom_net_system_tray = XInternAtom(_display, buf, 0);
	}
	
	return X11_atom_net_system_tray;
}

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define OPCODE "_NET_SYSTEM_TRAY_OPCODE"

Window X11_get_system_tray()
{
	return XGetSelectionOwner(_display, get_net_system_tray());
}

bool X11_window_dock(Window window)
{
	Window xmanager=None;
	XClientMessageEvent ev;
	Atom OpCodeAtom;

	XGrabServer(_display);

	xmanager = X11_get_system_tray();
	if (xmanager != None)
		XSelectInput(_display, xmanager, StructureNotifyMask);

	XUngrabServer(_display);
	XFlush(_display);
	
	if (xmanager == None)
		return TRUE;

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
	usleep(10000);
	
	return FALSE;
}

void X11_flush()
{
	XFlush(_display);
}
