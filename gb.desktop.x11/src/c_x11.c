/***************************************************************************

  c_x11.c

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

#define __C_X11_C

#include "x11.h"
#include "c_x11.h"

//#define DEBUG_ICON 1

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

BEGIN_METHOD(X11_FindWindow, GB_STRING title; GB_STRING klass; GB_STRING role)

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
		
		if (ltitle)
		{
			X11_get_window_title(win, &prop, &lprop);
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

		*((int *)GB.Array.Add(result)) = (int)win;
	}

	GB.ReturnObject(result);

END_METHOD


BEGIN_METHOD(X11_SendKey, GB_STRING key; GB_BOOLEAN press)

	char *error;
	
	if (X11_init())
		return;

	error = X11_send_key(GB.ToZeroString(ARG(key)), VARG(press));
	if (error) GB.Error(error);

END_METHOD


BEGIN_PROPERTY(X11_RootWindow)

	if (X11_init())
		return;

	GB.ReturnInteger(X11_root);

END_PROPERTY


BEGIN_PROPERTY(X11_Time)

	intptr_t time;
	
	GB.Component.GetInfo("TIME", POINTER(&time));
	
	GB.ReturnInteger((int)time);

END_PROPERTY


BEGIN_METHOD(X11_GetWindowProperty, GB_INTEGER window; GB_STRING name)

	char *value;
	Atom type;
	Atom prop;
	int format;
	GB_ARRAY array;
	int i, count;
	char *name;
	Window window;
	GB_VARIANT_VALUE ret;
	
	if (X11_init())
		return;
	
	prop = X11_intern_atom(GB.ToZeroString(ARG(name)), FALSE);
	if (prop == None)
	{
		GB.ReturnVariant(NULL);
		return;
	}
	
	window = VARG(window);
	
	value = X11_get_property(window, prop, &type, &format, &count);
	if (!value)
	{
		GB.ReturnVariant(NULL);
		return;
	}
	
	if (type == XA_ATOM)
	{
		GB.Array.New(&array, GB_T_STRING, count);
		for (i = 0; i < count; i++)
		{
			name = XGetAtomName(X11_display, *((Atom *)value + i));
			*(char **)GB.Array.Get(array, i) = GB.NewZeroString(name);
			XFree(name);
		}
		
		ret.type = GB_T_OBJECT;
		ret.value._object = array;
		GB.ReturnVariant(&ret);
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
				*(char **)GB.Array.Add(array) = GB.NewString(value, i);
			i++;
			value += i;
			count -= i;
		}
		ret.type = GB_T_OBJECT;
		ret.value._object = array;
		GB.ReturnVariant(&ret);
	}
	else
	{
		switch(format)
		{
			case 16:
				count = GB.StringLength(value) / sizeof(int);
				GB.Array.New(&array, GB_T_SHORT, count);
				/*for (i = 0; i < count; i++)
					*((short *)GB.Array.Get(array, i)) = *((short *)value + i);*/
				memcpy(GB.Array.Get(array, 0), value, sizeof(short) * count);

				ret.type = GB_T_OBJECT;
				ret.value._object = array;
				GB.ReturnVariant(&ret);
				
				break;
				
			case 32: // A "long", not necessarily 32 bits!!
				count = GB.StringLength(value) / sizeof(long);
				GB.Array.New(&array, GB_T_INTEGER, count);
				for (i = 0; i < count; i++)
					*((int *)GB.Array.Get(array, i)) = *((long *)value + i);
				
				ret.type = GB_T_OBJECT;
				ret.value._object = array;
				GB.ReturnVariant(&ret);
				
				break;
				
			default:
				
				ret.type = GB_T_STRING;
				ret.value._string = value;
				GB.ReturnVariant(&ret);
		}
	}

END_METHOD

BEGIN_METHOD(X11_SetWindowProperty, GB_INTEGER window; GB_STRING name; GB_STRING type; GB_VARIANT value)

	Atom type;
	Atom prop;
	int format;
	int count;
	void *data;
	void *object;
	Window window;
	void *buffer = NULL;
	#if OS_64BITS
	long padded_value;
	long *padded_data;
	int i;
	#endif
	
	if (X11_init())
		return;
	
	prop = X11_intern_atom(GB.ToZeroString(ARG(name)), TRUE);
	type = X11_intern_atom(GB.ToZeroString(ARG(type)), TRUE);
	count = 0;
	format = 0;
	
	window = VARG(window);
	
	switch(VARG(value).type)
	{
		case GB_T_STRING:
		case GB_T_CSTRING:
			format = 8;
			data = VARG(value).value._string;
			count = GB.StringLength(data);
			break;
			
		case GB_T_BOOLEAN:
		case GB_T_BYTE:
		case GB_T_SHORT:
		case GB_T_INTEGER:
			format = 32;
			#if OS_64BITS
			padded_value = VARG(value).value._integer;
			data = &padded_value;
			#else
			data = &VARG(value).value._integer;
			#endif
			count = 1;
			break;
		
		default:
			if (VARG(value).type >= GB_T_OBJECT)
			{
				object = VARG(value).value._object;
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
							#if OS_64BITS
							padded_data = (long *)alloca(sizeof(long) * count);
							for (i = 0; i < count; i++)
								padded_data[i] = *((int *)data + i);
							data = padded_data;
							#endif
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
								
								break;
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

	GB.Error("Datatype not supported");

END_METHOD

BEGIN_METHOD(X11_InternAtom, GB_STRING atom; GB_BOOLEAN create)

	if (X11_init())
		return;
	
	GB.ReturnInteger(X11_intern_atom(GB.ToZeroString(ARG(atom)), VARGOPT(create, FALSE)));

END_METHOD

BEGIN_METHOD(X11_GetAtomName, GB_INTEGER atom)

	char *name;

	if (X11_init())
		return;
		
	name = XGetAtomName(X11_display, VARG(atom));
	GB.ReturnNewZeroString(name);
	XFree(name);

END_METHOD

BEGIN_METHOD(X11_SendClientMessage, GB_STRING message; GB_OBJECT data; GB_INTEGER window)

	GB_ARRAY array;
	void *data = NULL;
	int count = 0;
	int format = 0;
	#if OS_64BITS
	long *padded_data;
	int i;
	#endif
	
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
				#if OS_64BITS
					padded_data = (long *)alloca(sizeof(long) * count);
					for (i = 0; i < count; i++)
						padded_data[i] = *((int *)data + i);
					data = padded_data;
				#endif
				break;
				
			/*case GB_T_STRING:
				format = 32;
				// TODO: convert strings to Atom
				break;*/
				
			default:
				fprintf(stderr, "gb.desktop: unsupported array datatype for 'Data' argument");
				return;
		}
	}
	
	X11_send_client_message(X11_root, VARGOPT(window, X11_root),
		X11_intern_atom(GB.ToZeroString(ARG(message)), TRUE),
		data, format, count);

END_METHOD

BEGIN_PROPERTY(X11_EventFilter)

	if (X11_init())
		return;
		
	if (READ_PROPERTY)
		GB.ReturnBoolean(X11_event_filter_enabled);
	else
		X11_enable_event_filter(VPROP(GB_BOOLEAN));

END_PROPERTY

BEGIN_METHOD(X11_WatchWindow, GB_INTEGER window; GB_BOOLEAN watch)

	XWindowAttributes attr;
	int mask = PropertyChangeMask | StructureNotifyMask;
	
	if (X11_init())
		return;
		
	XGetWindowAttributes(X11_display, VARG(window), &attr);

	if (VPROP(GB_BOOLEAN))
		XSelectInput(X11_display, (Window)VARG(window), attr.your_event_mask | mask);
	else
		XSelectInput(X11_display, (Window)VARG(window), attr.your_event_mask & ~mask);

END_METHOD

BEGIN_METHOD(X11_GetWindowGeometry, GB_INTEGER window)

	GB_ARRAY array;
	int *data;

	if (X11_init())
		return;
		
	GB.Array.New(&array, GB_T_INTEGER, 4);
	data = (int *)GB.Array.Get(array, 0);
	
	X11_get_window_geometry(VARG(window), &data[0], &data[1], &data[2], &data[3]);
	//XGetGeometry(X11_display, VARG(window), &root, &data[0], &data[1], (uint *)&data[2], (uint *)&data[3], &ignore, &ignore);
	GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(X11_MakeIcon, GB_OBJECT data; GB_INTEGER width; GB_INTEGER height)

	GB_ARRAY array;
	int *data;
	int count;
	int size;
	int width = VARGOPT(width, -1);
	int height = VARGOPT(height, width);
	int w, h;
	
	array = VARG(data);
	if (GB.CheckObject(array))
		return;
	
	data = (int *)GB.Array.Get(array, 0);
	count = GB.Array.Count(array);
	
	if (width < 0)
	{
		for(;;)
		{
			if (count < 2)
				break;
			w = data[0];
			h = data[1];
			if (!w || !h)
				break;
			#if DEBUG_ICON
			fprintf(stderr, "%d [%d %d]\n", count, w, h);
			#endif
			if (w > width)
			{
				width = w;
				height = h;
			}
			size = w * h + 2;
			data += size;
			count -= size;
		}

		data = (int *)GB.Array.Get(array, 0);
		count = GB.Array.Count(array);
	}
	
	for(;;)
	{
		if (count < 2)
		{
			GB.ReturnNull();
			return;
		}
		w = data[0];
		h = data[1];
		if (w == width && h == height)
			break;
		if (!w || !h)
			GB.ReturnNull();
		size = w * h + 2;
		data += size;
		count -= size;
	}
	
	GB.ReturnObject(IMAGE.Create(w, h, GB_IMAGE_BGRA, (unsigned char *)&data[2]));

END_METHOD

BEGIN_METHOD(X11_MinimizeWindow, GB_INTEGER window; GB_BOOLEAN minimized)

	if (X11_init())
		return;
	
	if (VARG(minimized))
	{
		long state = IconicState;
		
		X11_send_client_message(X11_root, VARG(window),
			X11_intern_atom("WM_CHANGE_STATE", TRUE),
			(char *)&state, 32, 1);
	}
	else
	{
		XMapWindow(X11_display, VARG(window));
	}

END_METHOD


BEGIN_METHOD_VOID(X11_Flush)

	if (X11_init())
		return;
	
	XFlush(X11_display);

END_METHOD


BEGIN_METHOD_VOID(X11_Sync)

	if (X11_init())
		return;
	
	XSync(X11_display, FALSE);

END_METHOD


BEGIN_METHOD(X11_MoveWindow, GB_INTEGER window; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	if (MISSING(w) || MISSING(h))
		XMoveWindow(X11_display, VARG(window), VARG(x), VARG(y));
	else
		XMoveResizeWindow(X11_display, VARG(window), VARG(x), VARG(y), VARG(w), VARG(h));

END_METHOD

BEGIN_METHOD(X11_ResizeWindow, GB_INTEGER window; GB_INTEGER w; GB_INTEGER h)

	XResizeWindow(X11_display, VARG(window), VARG(w), VARG(h));

END_METHOD



GB_DESC X11Desc[] =
{
  GB_DECLARE("X11", 0), GB_VIRTUAL_CLASS(),
  
  //GB_STATIC_METHOD("Init", NULL, CDESKTOP_init, NULL),
  
  GB_CONSTANT("None", "i", None),
  GB_CONSTANT("CurrentTime", "i", CurrentTime),

	GB_STATIC_METHOD("FindWindow", "Integer[]", X11_FindWindow, "[(Title)s(Application)s(Role)s]"),
	GB_STATIC_METHOD("SendKey", NULL, X11_SendKey, "(Key)s(Press)b"),
	GB_STATIC_PROPERTY_READ("RootWindow", "i", X11_RootWindow),
	GB_STATIC_PROPERTY_READ("Time", "i", X11_Time),
	GB_STATIC_METHOD("GetWindowProperty", "v", X11_GetWindowProperty, "(Window)i(Property)s"),
	GB_STATIC_METHOD("SetWindowProperty", NULL, X11_SetWindowProperty, "(Window)i(Property)s(Type)s(Value)v"),
	GB_STATIC_METHOD("InternAtom", "i", X11_InternAtom, "(Atom)s[(Create)b]"),
	GB_STATIC_METHOD("GetAtomName", "s", X11_GetAtomName, "(Atom)i"),
	GB_STATIC_METHOD("SendClientMessageToRootWindow", NULL, X11_SendClientMessage, "(Message)s[(Data)Array;(Window)i]"),
  GB_STATIC_PROPERTY("EventFilter", "b", X11_EventFilter),
  GB_STATIC_METHOD("WatchWindow", NULL, X11_WatchWindow, "(Window)i(Watch)b"),
  GB_STATIC_METHOD("GetWindowGeometry", "Integer[]", X11_GetWindowGeometry, "(Window)i"),
  GB_STATIC_METHOD("MakeIcon", "Image", X11_MakeIcon, "(Data)Array;[(Width)i(Height)h]"),
  GB_STATIC_METHOD("MinimizeWindow", NULL, X11_MinimizeWindow, "(Window)i(Minimized)b"),
  GB_STATIC_METHOD("Sync", NULL, X11_Sync, NULL),
  GB_STATIC_METHOD("Flush", NULL, X11_Flush, NULL),
  GB_STATIC_METHOD("MoveWindow", NULL, X11_MoveWindow, "(Window)i(X)i(Y)i[(Width)i(Height)i]"),
  GB_STATIC_METHOD("ResizeWindow", NULL, X11_ResizeWindow, "(Window)i(Width)i(Height)i"),

  GB_END_DECLARE
};

//---- X11Watcher ---------------------------------------------------------

static CX11WATCHER *_watcher_list = NULL;

void WATCHER_event_filter(XEvent *e)
{
	CX11WATCHER *watcher;
	
	if (!_watcher_list)
		return;

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

BEGIN_METHOD(X11Watcher_new, GB_INTEGER window; GB_STRING property)

	if (X11_init())
		return;
		
	WATCHER->window = VARGOPT(window, 0);
	WATCHER->property = MISSING(property) ? 0 : X11_intern_atom(GB.ToZeroString(ARG(property)), FALSE);
	
	if (!_watcher_list)
		X11_enable_event_filter(TRUE);
	
	LIST_insert(&_watcher_list, WATCHER, &WATCHER->list);

END_METHOD

BEGIN_METHOD_VOID(X11Watcher_free)

	LIST_remove(&_watcher_list, WATCHER, &WATCHER->list);
	if (!_watcher_list)
		X11_enable_event_filter(FALSE);

END_METHOD

GB_DESC X11WatcherDesc[] =
{
  GB_DECLARE("X11Watcher", sizeof(CX11WATCHER)),
  
  GB_METHOD("_new", NULL, X11Watcher_new, "[(Window)i(Property)s]"),
  GB_METHOD("_free", NULL, X11Watcher_free, NULL),
  
  GB_EVENT("Property", NULL, "(Window)i(Property)i", &EVENT_Change),
  GB_EVENT("Configure", NULL, "(Window)i(X)i(Y)i(Width)i(Height)i", &EVENT_Window),
  
  GB_END_DECLARE
};
