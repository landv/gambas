/***************************************************************************

  desktop.c

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

static bool _desktop_done = FALSE;
static char _desktop[32];

static const char *calc_desktop_type()
{
	char *env;
	
	env = getenv("KDE_FULL_SESSION");
	if (env && strcasecmp(env, "true") == 0)
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
	
	env = getenv("XDG_CURRENT_DESKTOP");
	if (env && *env && strlen(env) < sizeof(_desktop))
	{
		if (env[0] == 'X' && env[1] == '-')
			env += 2;
		
		return env;
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
	
	env = getenv("DESKTOP_SESSION");
	if (env && strcasecmp(env, "XFCE") == 0)
		return "XFCE";
	
	env = getenv("XDG_MENU_PREFIX");
	if (env && strncasecmp(env, "XFCE", 4) == 0)
		return "XFCE";
	
	env = getenv("XDG_DATA_DIRS");
	if (env && strstr(env, "/xfce"))
		return "XFCE";
	
	return "";
}

const char *DESKTOP_get_type()
{
	const char *type;
	char *p;
	
	if (!_desktop_done)
	{
		type = calc_desktop_type();
		p = _desktop;
		
		while ((*p++ = toupper(*type++)));
		_desktop_done = TRUE;
	}
	
	return _desktop;
}
