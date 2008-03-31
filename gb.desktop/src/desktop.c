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


BEGIN_METHOD(CDESKTOP_get_window_property, GB_POINTER id; GB_STRING name)

	char *value;
	Atom type;
	Atom prop;
	int format;
	GB_ARRAY array;
	int i, count;
	char *name;
	
	if (X11_init())
		return;
	
	prop = X11_intern_atom(GB.ToZeroString(ARG(name)), FALSE);
	if (prop == None)
	{
		GB.ReturnNull();
		return;
	}
	
	value = X11_get_property((Window)VARG(id), prop, &type, &format);
	
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
				
			case 32:
				count = GB.StringLength(value) / 4;
				GB.Array.New(&array, GB_T_INTEGER, count);
				for (i = 0; i < count; i++)
					*((int *)GB.Array.Get(array, i)) = *((int *)value + i);
				GB.ReturnObject(array);
				break;
				
			default:
				GB.ReturnString(value);
		}
	}

END_METHOD

BEGIN_METHOD(CDESKTOP_set_window_property, GB_POINTER id; GB_STRING name; GB_STRING type; GB_VARIANT value)

	char *value;
	Atom type;
	Atom prop;
	int format;
	GB_ARRAY array;
	int i;
	int count;
	void *data;
	void *object;
	
	if (X11_init())
		return;
	
	prop = X11_intern_atom(GB.ToZeroString(ARG(name)), TRUE);
	type = X11_intern_atom(GB.ToZeroString(ARG(type)), TRUE);
	count = 0;
	format = 0;
	
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
			
		case GB_T_OBJECT:
		
			object = VARG(value)._object.value;
			if (GB.Is(object, GB.FindClass("Array")))
			{
				data = GB.Array.Get((GB_ARRAY)object, 0);
				count = GB.Array.Count((GB_ARRAY)object);
				
				if (GB.Is(object, GB.FindClass("String[]")))
				{
				 // TODO: convert String[] to Integer[] of atoms
				}
				else if (GB.Is(object, GB.FindClass("Byte[]")))
					format = 8;
				else if (GB.Is(object, GB.FindClass("Short[]")))
					format = 16;
				else if (GB.Is(object, GB.FindClass("Integer[]")))
					format = 32;
			}
			
			break;
		
	}
	
	if (format)
		X11_set_property(VARG(id), prop, type, format, data, count);

END_METHOD

BEGIN_METHOD(CDESKTOP_intern_atom, GB_STRING atom; GB_BOOLEAN create)

	if (X11_init())
		return;
	GB.ReturnInteger(X11_intern_atom(GB.ToZeroString(ARG(atom)), !VARGOPT(create, FALSE)));

END_METHOD

BEGIN_METHOD(CDESKTOP_send_client_message, GB_STRING message; GB_OBJECT data)

	GB_ARRAY array;
	char *data = NULL;
	int count = 0;
	int format = 0;
	
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
				
			case GB_T_STRING:
				format = 32;
				// TODO: convert strings to Atom
				break;
				
			default:
				return;
		}
	}
	
	X11_send_client_message(X11_root, 
		X11_intern_atom(GB.ToZeroString(ARG(message)), TRUE),
		data, format, count);
		

END_METHOD

/*GB_DESC CDesktopWindowDesc[] =
{
  GB_DECLARE("._Desktop.Window", 0), GB_VIRTUAL_CLASS(),
  
  GB_STATIC_METHOD("GetProperty", "v", CDESKTOPWINDOW_get_property, "(Property)s(Type)i"),
  GB_STATIC_METHOD("SetProperty", NULL, CDESKTOPWINDOW_set_property, "(Property)s(Type)i(Value)v"),
  
  GB_END_DECLARE
};*/

GB_DESC CDesktopDesc[] =
{
  GB_DECLARE("_Desktop", 0), GB_VIRTUAL_CLASS(),
  
  //GB_STATIC_METHOD("_init", NULL, CDESKTOP_init, NULL),
  
	GB_STATIC_METHOD("FindWindow", "Integer[]", CDESKTOP_find, "[(Title)s(Application)s(Role)s]"),
	GB_STATIC_METHOD("SendKey", NULL, CDESKTOP_sendkey, "(Key)s(Press)b"),
	GB_STATIC_PROPERTY_READ("RootWindow", "i", CDESKTOP_root),
	GB_STATIC_METHOD("GetWindowProperty", "v", CDESKTOP_get_window_property, "(Window)i(Property)s"),
	//GB_STATIC_METHOD("GetWindowPropertyType", "s", CDESKTOP_get_window_property_type, "(Window)i(Property)s"),
	GB_STATIC_METHOD("SetWindowProperty", NULL, CDESKTOP_set_window_property, "(Window)i(Property)s(Type)s(Value)v"),
	GB_STATIC_METHOD("InternAtom", "i", CDESKTOP_intern_atom, "(Atom)s[(Create)b]"),
	GB_STATIC_METHOD("SendClientMessageToRootWindow", NULL, CDESKTOP_send_client_message, "(Message)s[(Data)Array;]"),
  //GB_STATIC_PROPERTY_SELF("Root", "._Desktop.Window"),
  //GB_STATIC_METHOD("_get", "._Desktop.Window"),
  
  GB_END_DECLARE
};

