/***************************************************************************

  main.c

  (c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __MAIN_C

#include "cfaketrayicon.h"
#include "main.h"

GB_INTERFACE GB EXPORT;

// Prevents gbi3 from complaining

GB_DESC *GB_CLASSES[] EXPORT =
{
  NULL
};

GB_DESC *GB_OPTIONAL_CLASSES[] EXPORT =
{
	FakeTrayIconDesc, FakeTrayIconsDesc,
	NULL
};

int EXPORT GB_INIT(void)
{
	return 0;
}

void EXPORT GB_AFTER_INIT(void)
{
	GB_FUNCTION func;
	bool has_dbus_systemtray = FALSE;
	void (*declare_tray_icon)();
	
	GB.Component.Load("gb.dbus");
	
	if (!GB.GetFunction(&func, (void *)GB.FindClass("DBus"), "_HasSystemTray", NULL, NULL))
		has_dbus_systemtray = GB.Call(&func, 0, FALSE)->_boolean.value;
	
	if (has_dbus_systemtray)
		GB.Component.Load("gb.dbus.trayicon");
	else
	{
		GB.Component.GetInfo("DECLARE_TRAYICON", POINTER(&declare_tray_icon));
		(*declare_tray_icon)();
	}
}

void EXPORT GB_EXIT()
{
}


