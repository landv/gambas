/***************************************************************************

  desktop.c

  (c) 2007 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __DESKTOP_C

#include "x11.h"
#include "desktop.h"

#if 0
typedef struct PROPERTY_FORMAT
{
	const char *name;
	Atom atom;
  const char *format;
};

static PROPERTY_FORMAT _property_format[] = {
	{"ARC"                    XA_ARC,             "16iiccii"         },
	{"ATOM",                  XA_ATOM,            "32a"              },
	{"BITMAP",                XA_BITMAP,          "32x"              },
	{"CARDINAL",              XA_CARDINAL,        "0c"               },
	{"COLORMAP",              XA_COLORMAP,        "32x"              },
	{"CURSOR",                XA_CURSOR,          "32x"              },
	{"DRAWABLE",              XA_DRAWABLE,        "32x"              },
	{"FONT",                  XA_FONT,            "32x"              },
	{"INTEGER",               XA_INTEGER,         "0i"               },
	{"PIXMAP",                XA_PIXMAP,          "32x"              },
	{"POINT",                 XA_POINT,           "16ii"             },
	{"RECTANGLE",             XA_RECTANGLE,       "16iicc"           },
	{"RGB_COLOR_MAP",         XA_RGB_COLOR_MAP,   "32xcccccccxx"     },
	{"STRING",                XA_STRING,          "8s"               },
	{"WINDOW",                XA_WINDOW,          "32x"              },
	{"VISUALID",              XA_VISUALID,        "32x"              },
	{"WM_COLORMAP_WINDOWS",   0,                  "32x"              },
	{"WM_COMMAND",            XA_WM_COMMAND,      "8s"               },
	{"WM_HINTS",              XA_WM_HINTS,        "32mbcxxiixx"      },
	{"WM_ICON_NAME",          XA_WM_ICON_NAME,    "8t"               },
	{"WM_ICON_SIZE",          XA_WM_ICON_SIZE,    "32cccccc"         },
	{"WM_NAME",               XA_WM_NAME,         "8t"               },
	{"WM_PROTOCOLS",          0,                  "32a"              },
	{"WM_SIZE_HINTS",         XA_WM_SIZE_HINTS,   "32mii"            },
	{"WM_STATE",              0,                  "32cx"             }
};
#endif

DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Window);

/*BEGIN_METHOD_VOID(CDESKTOP_init)

	X11_init();

END_METHOD*/

BEGIN_METHOD(CDESKTOP_find, GB_STRING title; GB_STRING klass; GB_STRING role)

	Window *windows;
	Window win;
	int count;
	int i;
	char *title = MISSING(title) ? NULL : STRING(title);
	int ltitle = MISSING(title) ? 0 : LENGTH(title);
	char *klass = MISSING(klass) ? NULL : STRING(klass);
	int lklass = MISSING(klass) ? 0 : LENGTH(klass);
	char *role = MISSING(role) ? NULL : STRING(role);
	int lrole = MISSING(role) ? 0 : LENGTH(role);
	char *prop;
	int lprop;
	GB_ARRAY result;

	if (X11_init())
		return;
	
	GB.Array.New(&result, GB_T_INTEGER, 0);

	X11_find_windows(&windows, &count);

	for (i = 0; i < count; i++)
	{
		win = windows[i];
		//qDebug("win = %08X", win);
		if (ltitle)
		{
			X11_get_window_title(win, &prop, &lprop);
			//qDebug("title = %.*s", lprop, prop);
			if (!GB.MatchString(title, ltitle, prop, lprop))
				continue;
		}

		if (lklass)
		{
			X11_get_window_class(win, &prop, &lprop);
			//qDebug("class = %.*s", lprop, prop);
			if (!GB.MatchString(klass, lklass, prop, lprop))
				continue;
		}

		if (lrole)
		{
			X11_get_window_role(win, &prop, &lprop);
			//qDebug("role = %.*s", lprop, prop);
			if (!GB.MatchString(role, lrole, prop, lprop))
				continue;
		}

		*((Window *)GB.Array.Add(result)) = win;
	}

	XFree(windows);

	GB.ReturnObject(result);

END_METHOD


BEGIN_METHOD(CDESKTOP_sendkey, GB_STRING key; GB_BOOLEAN press)

	char *error;
	
	if (X11_init())
		return;

	error = X11_send_key(GB.ToZeroString(ARG(key)), VARG(press));
	if (error) GB.Error(error);

END_METHOD


BEGIN_PROPERTY(CDESKTOP_root)

	if (X11_init())
		return;
	
	GB.ReturnPointer((void *)X11_root);

END_PROPERTY


BEGIN_METHOD(CDESKTOP_get_window_property, GB_STRING name; GB_INTEGER window)

	char *value;
	Atom type;
	Atom prop;
	int format;
	GB_ARRAY array;
	int i, count;
	char *name;
	Window window;
	
	if (X11_init())
		return;
	
	prop = X11_intern_atom(GB.ToZeroString(ARG(name)), FALSE);
	if (prop == None)
	{
		GB.ReturnNull();
		return;
	}
	
	window = VARGOPT(window, X11_root);
	
	value = X11_get_property(window, prop, &type, &format);
	if (!value)
	{
		GB.ReturnNull();
		return;
	}
	
	/*name = XGetAtomName(X11_display, type);
	fprintf(stderr, "type = %s\n", name);
	XFree(name);*/
	
	if (type == XA_ATOM)
	{
		count = GB.StringLength(value) / sizeof(Atom);
		GB.Array.New(&array, GB_T_STRING, count);
		for (i = 0; i < count; i++)
		{
			name = XGetAtomName(X11_display, *((Atom *)value + i));
			GB.NewString((char **)GB.Array.Get(array, i), name, 0);
			XFree(name);
		}
		GB.ReturnObject(array);
	}
	else if (type == X11_UTF8_STRING && format == 8)
	{
		count = GB.StringLength(value);
		GB.Array.New(&array, GB_T_STRING, 0);
		while (count > 0)
		{
			for (i = 0; i < count; i++)
			{
				if (!value[i])
					break;
			}
			if (i)
				GB.NewString((char **)GB.Array.Add(array), value, i);
			i++;
			value += i;
			count -= i;
		}
		GB.ReturnObject(array);
	}
	else
	{
		switch(format)
		{
			case 16:
				count = GB.StringLength(value) / 2;
				GB.Array.New(&array, GB_T_SHORT, count);
				for (i = 0; i < count; i++)
					*((short *)GB.Array.Get(array, i)) = *((short *)value + i);
				GB.ReturnObject(array);
				break;
				
			case 32: // A "long", not necessarily 32 bits!!
				count = GB.StringLength(value) / 4;
				GB.Array.New(&array, GB_T_INTEGER, count);
				for (i = 0; i < count; i++)
					*((int *)GB.Array.Get(array, i)) = *((long *)value + i);
				GB.ReturnObject(array);
				break;
				
			default:
				GB.ReturnString(value);
		}
	}

END_METHOD

BEGIN_METHOD(CDESKTOP_set_window_property, GB_STRING name; GB_STRING type; GB_VARIANT value; GB_INTEGER window)

	Atom type;
	Atom prop;
	int format;
	int count;
	void *data;
	void *object;
	Window window;
	void *buffer = NULL;
	
	if (X11_init())
		return;
	
	prop = X11_intern_atom(GB.ToZeroString(ARG(name)), TRUE);
	type = X11_intern_atom(GB.ToZeroString(ARG(type)), TRUE);
	count = 0;
	format = 0;
	
	window = VARGOPT(window, X11_root);
	
	switch(VARG(value).type)
	{
		case GB_T_STRING:
		case GB_T_CSTRING:
			format = 8;
			data = VARG(value)._string.value;
			count = GB.StringLength(data);
			break;
			
		case GB_T_BOOLEAN:
		case GB_T_BYTE:
		case GB_T_SHORT:
		case GB_T_INTEGER:
			format = 32;
			data = &VARG(value)._integer.value;
			count = 1;
			break;
		
		default:
			if (VARG(value).type >= GB_T_OBJECT)
			{
				object = VARG(value)._object.value;
				if (GB.Is(object, GB.FindClass("Array")))
				{
					data = GB.Array.Get((GB_ARRAY)object, 0);
					count = GB.Array.Count((GB_ARRAY)object);
					
					switch((int)GB.Array.Type(object))
					{
						case GB_T_BYTE:
							format = 8;
							break;
							
						case GB_T_SHORT:
							format = 16;
							break;
							
						case GB_T_INTEGER:
							format = 32;
							break;
							
						case GB_T_STRING:
							if (type == X11_UTF8_STRING)
							{
								int i, len;
								char *p;
								char **d = (char **)data;
								
								len = 0;
								for (i = 0; i < count; i++)
									len += GB.StringLength(d[i]) + 1;
									
								GB.Alloc(&buffer, len);
								
								format = 8;
								data = buffer;
								count = len;
								
								p = buffer;
								for (i = 0; i < count; i++)
								{
									len = GB.StringLength(d[i]);
									memcpy(p, d[i], len + 1);
									p += len + 1;
								}
							}
						default:
							goto __ERROR;
					}
				}
			}
			else
				goto __ERROR;
			
			break;
	}
	
	if (format)
		X11_set_property(window, prop, type, format, data, count);
		
	if (buffer)
		GB.Free(&buffer);
	
	return;
	
__ERROR:

	GB.Error("Cannot deal with that datatype");

END_METHOD

BEGIN_METHOD(CDESKTOP_intern_atom, GB_STRING atom; GB_BOOLEAN create)

	if (X11_init())
		return;
	GB.ReturnInteger(X11_intern_atom(GB.ToZeroString(ARG(atom)), !VARGOPT(create, FALSE)));

END_METHOD

BEGIN_METHOD(CDESKTOP_get_atom_name, GB_INTEGER atom)

	char *name;

	if (X11_init())
		return;
		
	name = XGetAtomName(X11_display, VARG(atom));
	GB.ReturnNewZeroString(name);
	XFree(name);

END_METHOD

BEGIN_METHOD(CDESKTOP_send_client_message, GB_STRING message; GB_OBJECT data; GB_INTEGER window)

	GB_ARRAY array;
	char *data = NULL;
	int count = 0;
	int format = 0;
	
	if (X11_init())
		return;
		
	if (!MISSING(data) && VARG(data))
	{
		array = VARG(data);
		data = GB.Array.Get(array, 0);
		count = GB.Array.Count(array);
		
		switch((int)GB.Array.Type(array))
		{
			case GB_T_BYTE:
				format = 8;
				break;
				
			case GB_T_SHORT:
				format = 16;
				break;
				
			case GB_T_INTEGER:
				format = 32;
				break;
				
			/*case GB_T_STRING:
				format = 32;
				// TODO: convert strings to Atom
				break;*/
				
			default:
				return;
		}
	}
	
	X11_send_client_message(X11_root, VARGOPT(window, X11_root),
		X11_intern_atom(GB.ToZeroString(ARG(message)), TRUE),
		data, format, count);
		

END_METHOD

BEGIN_PROPERTY(CDESKTOP_event_filter)

	if (X11_init())
		return;
		
	if (READ_PROPERTY)
		GB.ReturnBoolean(X11_event_filter_enabled);
	else
		X11_enable_event_filter(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD(CDESKTOP_watch_window, GB_INTEGER window; GB_BOOLEAN watch)

	XWindowAttributes attr;
	int mask = PropertyChangeMask | StructureNotifyMask;
	
	if (X11_init())
		return;
		
	XGetWindowAttributes(X11_display, VARG(window), &attr);

	if (VPROP(GB_BOOLEAN))
		XSelectInput(X11_display, VARG(window), attr.your_event_mask | mask);
	else
		XSelectInput(X11_display, VARG(window), attr.your_event_mask & ~mask);

END_METHOD

BEGIN_METHOD(CDESKTOP_get_window_geometry, GB_INTEGER window)

	GB_ARRAY array;
	int *data;
	Window root;
	uint ignore;

	if (X11_init())
		return;
		
	GB.Array.New(&array, GB_T_INTEGER, 4);
	data = (int *)GB.Array.Get(array, 0);
	
	XGetGeometry(X11_display, VARG(window), &root, &data[0], &data[1], (uint *)&data[2], (uint *)&data[3], &ignore, &ignore);
	GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(CDESKTOP_make_icon, GB_OBJECT data)

	GB_ARRAY array;
	int *data;
	
	array = VARG(data);
	if (GB.CheckObject(array))
		return;
	
	data = (int *)GB.Array.Get(array, 0);
	
	GB.ReturnObject(IMAGE.Create(data[0], data[1], GB_IMAGE_BGRA, (unsigned char *)&data[2]));

END_METHOD

BEGIN_METHOD(CDESKTOP_minimize_window, GB_INTEGER window; GB_BOOLEAN minimized)

	if (VARG(minimized))
	{
		int state = IconicState;
		
		X11_send_client_message(X11_root, VARG(window),
			X11_intern_atom("WM_CHANGE_STATE", TRUE),
			(char *)&state, 32, 1);
	}
	else
	{
		XMapWindow(X11_display, VARG(window));
	}

END_METHOD

GB_DESC CDesktopDesc[] =
{
  GB_DECLARE("_Desktop", 0), GB_VIRTUAL_CLASS(),
  
  //GB_STATIC_METHOD("Init", NULL, CDESKTOP_init, NULL),
  
  GB_CONSTANT("CurrentTime", "i", CurrentTime),
  
	GB_STATIC_METHOD("FindWindow", "Integer[]", CDESKTOP_find, "[(Title)s(Application)s(Role)s]"),
	GB_STATIC_METHOD("SendKey", NULL, CDESKTOP_sendkey, "(Key)s(Press)b"),
	GB_STATIC_PROPERTY_READ("RootWindow", "i", CDESKTOP_root),
	GB_STATIC_METHOD("GetWindowProperty", "v", CDESKTOP_get_window_property, "(Property)s[(Window)i]"),
	//GB_STATIC_METHOD("GetWindowPropertyType", "s", CDESKTOP_get_window_property_type, "(Window)i(Property)s"),
	GB_STATIC_METHOD("SetWindowProperty", NULL, CDESKTOP_set_window_property, "(Property)s(Type)s(Value)v[(Window)i]"),
	GB_STATIC_METHOD("InternAtom", "i", CDESKTOP_intern_atom, "(Atom)s[(Create)b]"),
	GB_STATIC_METHOD("GetAtomName", "s", CDESKTOP_get_atom_name, "(Atom)i"),
	GB_STATIC_METHOD("SendClientMessageToRootWindow", NULL, CDESKTOP_send_client_message, "(Message)s[(Data)Array;(Window)i]"),
  //GB_STATIC_PROPERTY_SELF("Root", "._Desktop.Window"),
  //GB_STATIC_METHOD("_get", "._Desktop.Window"),
  GB_STATIC_PROPERTY("EventFilter", "b", CDESKTOP_event_filter),
  GB_STATIC_METHOD("WatchWindow", NULL, CDESKTOP_watch_window, "(Window)i(Watch)b"),
  GB_STATIC_METHOD("GetWindowGeometry", "Integer[]", CDESKTOP_get_window_geometry, "(Window)i"),
  GB_STATIC_METHOD("MakeIcon", "Image", CDESKTOP_make_icon, "(Data)Array;"),
  GB_STATIC_METHOD("MinimizeWindow", NULL, CDESKTOP_minimize_window, "(Window)i(Minimized)b"),
  
  GB_END_DECLARE
};

/****************************************************************************

	_DesktopWatcher

****************************************************************************/

static CDESKTOPWATCHER *_watcher_list = NULL;

static void x11_event_filter(XEvent *e)
{
	CDESKTOPWATCHER *watcher;
	
	if (e->type == PropertyNotify)
	{
		LIST_for_each(watcher, _watcher_list)
		{
			if (watcher->window && e->xany.window != watcher->window)
				continue;
			if (watcher->property && e->xproperty.atom != watcher->property)
				continue;
					
			GB.Raise(watcher, EVENT_Change, 2, GB_T_INTEGER, e->xany.window, GB_T_INTEGER, e->xproperty.atom);
		}
	}
	else if (e->type == ConfigureNotify)
	{
		LIST_for_each(watcher, _watcher_list)
		{
			if (watcher->window && e->xany.window != watcher->window)
				continue;
			
			GB.Raise(watcher, EVENT_Window, 
				5, GB_T_INTEGER, e->xany.window,
				GB_T_INTEGER, e->xconfigure.x,
				GB_T_INTEGER, e->xconfigure.y,
				GB_T_INTEGER, e->xconfigure.width,
				GB_T_INTEGER, e->xconfigure.height);
		}
	}
}

static void enable_event_filter(bool enable)
{
	void (*set_event_filter)(void *) = NULL;
	
	GB.GetComponentInfo("SET_EVENT_FILTER", POINTER(&set_event_filter));
	if (set_event_filter)
		(*set_event_filter)(enable ? x11_event_filter : 0);
}

BEGIN_METHOD(CDESKTOPWATCHER_new, GB_INTEGER window; GB_STRING property)

	if (X11_init())
		return;
		
	WATCHER->window = VARGOPT(window, 0);
	WATCHER->property = MISSING(property) ? 0 : X11_intern_atom(GB.ToZeroString(ARG(property)), FALSE);
	
	if (!_watcher_list)
		enable_event_filter(TRUE);
	
	LIST_insert(&_watcher_list, WATCHER, &WATCHER->list);

END_METHOD

BEGIN_METHOD_VOID(CDESKTOPWATCHER_free)

	LIST_remove(&_watcher_list, WATCHER, &WATCHER->list);
	if (!_watcher_list)
		enable_event_filter(FALSE);

END_METHOD

GB_DESC CDesktopWatcherDesc[] =
{
  GB_DECLARE("_DesktopWatcher", sizeof(CDESKTOPWATCHER)),
  
  GB_METHOD("_new", NULL, CDESKTOPWATCHER_new, "[(Window)i(Property)s]"),
  GB_METHOD("_free", NULL, CDESKTOPWATCHER_free, NULL),
  
  GB_EVENT("Change", NULL, "(Window)i(Property)i", &EVENT_Change),
  GB_EVENT("Window", NULL, "(Window)i(X)i(Y)i(Width)i(Height)i", &EVENT_Window),
  
  GB_END_DECLARE
};
