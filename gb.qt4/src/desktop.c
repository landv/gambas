/***************************************************************************

  desktop.c

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

#define __DESKTOP_C

#include "desktop.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const GB_INTERFACE *GB_PTR;
#define GB (*GB_PTR)

#ifdef __cplusplus
}
#endif

static const char *_desktop;

static const char *calc_desktop_type()
{
	char *env;
	
	env = getenv("KDE_FULL_SESSION");
	if (env && strcmp(env, "true") == 0)
	{
		env = getenv("KDE_SESSION_VERSION");
		if (env)
		{
			if (strcmp(env, "4") == 0)
				return "KDE4";
			if (strcmp(env, "5") == 0)
				return "KDE5";
		}
		return "KDE";
	}
	
	env = getenv("GNOME_DESKTOP_SESSION_ID");
	if (env && *env)
		return "GNOME";
	
	env = getenv("MATE_DESKTOP_SESSION_ID");
	if (env && *env)
		return "MATE";
	
  env = getenv("E_BIN_DIR");
	if (env && *env)
	{
		env = getenv("E_LIB_DIR");
		if (env && *env)
			return "ENLIGHTENMENT";
	}
	
	env = getenv("WMAKER_BIN_NAME");
	if (env && *env)
		return "WINDOWMAKER";
	
	env = getenv("XDG_CURRENT_DESKTOP");
	if (env && *env)
	{
		if (strcasecmp(env, "LXDE") == 0)
			return "LXDE";
		if (strcasecmp(env, "UNITY") == 0)
			return "UNITY";
	}
	
	return NULL;
}

const char *DESKTOP_get_type()
{
	if (!_desktop)
	{
		const char *type = calc_desktop_type();
		if (!type)
			type = "?";
		_desktop = type;
	}
	
	return _desktop;
}

bool DESKTOP_load_trayicon_component()
{
	const char *desktop = DESKTOP_get_type();
	
	if (strcmp(desktop, "KDE4") == 0 || strcmp(desktop, "KDE5") == 0 || strcmp(desktop, "UNITY") == 0)
	{
		GB.Component.Load("gb.dbus");
		GB.Component.Load("gb.dbus.trayicon");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}