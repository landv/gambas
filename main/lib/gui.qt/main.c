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

#include "main.h"

enum { USE_NOTHING, USE_GB_QT4, USE_GB_QT5 };

const GB_INTERFACE *GB_PTR EXPORT;

// Prevents gbi3 from complaining

GB_DESC *GB_CLASSES[] EXPORT =
{
  NULL
};

char *GB_INCLUDE EXPORT = "gb.qt4|gb.qt5";


const char *get_name(int use)
{
	switch (use)
	{
		case USE_GB_QT4: return "gb.qt4";
		default: return "gb.qt5";
	}
}


static bool can_use(int use)
{
	static const char *ext[] = { "ext", "webkit", "opengl", NULL };
	char test[32];
	char *suffix;
	const char **pext;
	const char *name;
	
	name = get_name(use);
	
	if (!GB.Component.CanLoadLibrary(name))
		return FALSE;

	strcpy(test, name);
	suffix = test + strlen(name);
	*suffix++ = '.';
	
	for (pext = ext; *pext; pext++)
	{
		strcpy(suffix, *pext);
		if (GB.Component.Exist(test) && !GB.Component.CanLoadLibrary(test))
			return FALSE;
	}
	
	return TRUE;
}


int EXPORT GB_INIT(void)
{
	int use = USE_NOTHING;
	int use_other = USE_NOTHING;
	char *env;
	const char *comp;

	env = getenv("GB_GUI");
	if (env && *env)
	{
		if (strcmp(env, "gb.qt4") == 0)
			use = USE_GB_QT4;
		else if (strcmp(env, "gb.qt5") == 0)
			use = USE_GB_QT5;
		else
			fprintf(stderr, "gb.gui.qt: warning: '%s' component not supported\n", env);
	}
	
	if (use == USE_NOTHING)
	{
		use = USE_GB_QT5;
		
		env = getenv("KDE_FULL_SESSION");
		
		if (env && !strcmp(env, "true"))
		{
			env = getenv("KDE_SESSION_VERSION");
			if (env)
			{
				if (strcmp(env, "4") == 0)
					use = USE_GB_QT4;
				else if (strcmp(env, "5") == 0)
					use = USE_GB_QT5;
			}
		}
	}

	if (!can_use(use))
	{
		if (use == USE_GB_QT4)
			use_other = USE_GB_QT5;
		else
			use_other = USE_GB_QT4;
		
		if (can_use(use_other))
		{
			fprintf(stderr, "gb.gui.qt: warning: '%s' component not found, using '%s' instead\n", get_name(use), get_name(use_other));
			use = use_other;
		}
		else
		{
			fprintf(stderr, "gb.gui.qt: error: unable to find any QT component\n");
			exit(1);
		}
	}
	
	comp = get_name(use);
	
	if (GB.Component.Load(comp))
	{
		fprintf(stderr, "gb.gui.qt: error: cannot load component '%s'\n", comp);
		exit(1);
	}
	else
	{
		env = getenv("GB_GUI_DEBUG");
		if (env && !strcmp(env, "0")) 
			fprintf(stderr, "gb.gui.qt: loading '%s'\n", comp);
	}
  
  setenv("GB_GUI", comp, TRUE);

  return 0;
}

void EXPORT GB_EXIT()
{
}

