/***************************************************************************

  main.c

  (c) 2000-2017 Benoît Minisini <gambas@users.sourceforge.net>

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

GB_INTERFACE GB EXPORT;

// Prevents gbi3 from complaining

GB_DESC *GB_CLASSES[] EXPORT =
{
  NULL
};

char *GB_INCLUDE EXPORT = "gb.qt4|gb.qt5";

int EXPORT GB_INIT(void)
{
	int use = USE_NOTHING;
	char *env;
	const char *comp;
	const char *comp2;

	env = getenv("GB_GUI");
	if (env && *env)
	{
		if (strcmp(env, "gb.qt4") == 0)
			use = USE_GB_QT4;
		else if (strcmp(env, "gb.qt5") == 0)
			use = USE_GB_QT5;
		else
			fprintf(stderr, "gb.gui: warning: '%s' component not supported\n", env);
	}
	
	if (use == USE_NOTHING)
	{
		use = USE_GB_QT4;
		
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

	switch (use)
	{
		case USE_GB_QT4: comp = "gb.qt4"; break;
		case USE_GB_QT5: comp = "gb.qt5"; break;
		default: comp = "gb.qt5"; break;
	}

	if (GB.Component.Load(comp))
	{
		comp2 = use == USE_GB_QT5 ? "gb.qt4" : "gb.qt5";

		if (GB.Component.Load(comp2))
		{
			fprintf(stderr, "gb.gui.qt: error: unable to find any QT component\n");
			exit(1);
		}

		fprintf(stderr, "gb.gui: warning: '%s' component not found, using '%s' instead\n", comp, comp2);
		comp = comp2;
	}
  
  setenv("GB_GUI", comp, TRUE);

  return 0;
}

void EXPORT GB_EXIT()
{
}


