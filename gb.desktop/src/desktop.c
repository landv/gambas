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
	
  GB.Array.New(&result, GB_T_POINTER, 0);

	X11_find_windows(&windows, &count);

	for (i = 0; i < count; i++)
	{
		win = windows[i];
		//qDebug("win = %08X", win);
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

GB_DESC CDesktopDesc[] =
{
  GB_DECLARE("_Desktop", 0), GB_VIRTUAL_CLASS(),
  
  GB_STATIC_METHOD("Find", "Pointer[]", CDESKTOP_find, "[(Title)s(Application)s(Role)s]"),
  GB_STATIC_METHOD("SendKey", NULL, CDESKTOP_sendkey, "(Key)s(Press)b"),
  
  GB_END_DECLARE
};

